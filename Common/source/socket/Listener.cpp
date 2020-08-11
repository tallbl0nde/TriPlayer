#include "Log.hpp"
#include "socket/Listener.hpp"
#include <sys/socket.h>

// Number of failures before recreating listening socket
#define FAILURES 10
// Number of failures before sleeping indefinitely
#define FATAL_FAILURES 20
// Connection queue size (one for app, one for overlay)
#define QUEUE_SIZE 2

namespace Socket {
    Listener::Listener(int p) {
        this->listening = false;
        this->listeningSocket = -1;
        this->port = p;
        this->createListeningSocket();
    }

    void Listener::createListeningSocket() {
        // Don't try to create another socket if we have one
        if (this->listeningSocket >= 0) {
            return;
        }

        // Get a socket
        this->listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (this->listeningSocket < 0) {
            Log::writeError("[SOCKET] Couldn't create listening socket: " + std::to_string(errno));
            return;
        }

        // Bind to port
        this->addr.sin_family = AF_INET;
        this->addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        this->addr.sin_port = htons(this->port);
        const int optVal = 1;
        setsockopt(this->listeningSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&optVal, sizeof(optVal));
        if (bind(this->listeningSocket, (struct sockaddr *)&this->addr, sizeof(this->addr)) != 0) {
            Log::writeError("[SOCKET] Error binding to address/port: " + std::to_string(errno));
            this->closeListeningSocket();
            return;
        }

        // Listen
        if (::listen(this->listeningSocket, QUEUE_SIZE) != 0) {
            Log::writeError("[SOCKET] Error listening: " + std::to_string(errno));
            this->closeListeningSocket();
            return;
        }

        // Successful
        this->listening = true;
        Log::writeSuccess("[SOCKET] Listening socket created successfully");
    }

    void Listener::closeListeningSocket() {
        this->listening = false;
        if (this->listeningSocket >= 0) {
            close(this->listeningSocket);
            this->listeningSocket = -1;
        }
    }

    bool Listener::isListening() {
        return this->listening;
    }

    bool Listener::hasTransferSocket() {
        std::scoped_lock<std::mutex> mtx(this->mutex);
        return (this->sockets.size() != 0);
    }

    Transfer * Listener::getTransferSocket() {
        // Return nullptr if there is none available
        std::scoped_lock<std::mutex> mtx(this->mutex);
        if (this->sockets.empty()) {
            return nullptr;
        }

        // Otherwise return top element
        Transfer * socket = this->sockets.front();
        this->sockets.pop();
        return socket;
    }

    void Listener::listen(std::atomic<bool> & stop) {
        // Number of succeeding failures (this thread will start sleeping after a lot of errors)
        size_t failures = 0;
        bool loggedFailure = false;

        // Repeated wait for a connection
        Log::writeInfo("[SOCKET] Waiting for a connection...");
        while (!stop) {
            // Sleep indefinitely if there's a lot of failures
            if (failures >= FATAL_FAILURES) {
                if (!loggedFailure) {
                    Log::writeError("[SOCKET] Sleeping indefinitely due to 20 failed attempts in a row...");
                    loggedFailure = true;
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;

            // Recreate listening socket after some failures
            } else if (failures == FAILURES) {
                this->closeListeningSocket();
                this->createListeningSocket();
            }

            // Accept a connection
            socklen_t size = sizeof(this->addr);
            int transferSocket = accept(this->listeningSocket, (struct sockaddr *)&this->addr, (socklen_t *)&size);

            // If the value is negative an error occurred
            if (transferSocket < 0) {
                // errno = 113 when waking from sleep, simply recreate the listening socket
                if (errno == 113) {
                    Log::writeInfo("[SOCKET] No route to host (expected if waking from sleep)");
                    this->closeListeningSocket();
                    this->createListeningSocket();

                // Otherwise ignore and listen again
                } else {
                    Log::writeError("[SOCKET] Unable to accept a connection: " + std::to_string(errno));
                    failures++;
                }

            // Otherwise create an object using this socket fd
            } else {
                std::scoped_lock<std::mutex> mtx(this->mutex);
                this->sockets.push(new Transfer(transferSocket));
                if (Log::loggingLevel() == Log::Level::Info) {
                    Log::writeInfo("[SOCKET] Created #" + std::to_string(this->sockets.size()) + " socket");
                }
            }
        }
    }

    Listener::~Listener() {
        // Close any unreturned transfer sockets
        while (!this->sockets.empty()) {
            delete this->sockets.front();
            this->sockets.pop();
        }

        // Close this socket
        this->closeListeningSocket();
    }
};