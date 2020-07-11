#include <cstring>
#include <errno.h>
#include <switch.h>
#include <sys/socket.h>
#include "Log.hpp"
#include "utils/Socket.hpp"

// Number of bytes to read on one call to read()
#define READ_BYTES 200
// Connection queue size
#define QUEUE_SIZE 1

Socket::Socket(int p) {
    this->lSocket = -1;
    this->tSocket = -1;
    this->port = p;
    this->createLSocket();
}

void Socket::createLSocket() {
    // Get socket
    this->lSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->lSocket < 0) {
        Log::writeError("[SOCKET] Unable to create listening socket: " + std::to_string(errno));
        return;
    }

    // Bind and listen
    this->addr.sin_family = AF_INET;
    this->addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    this->addr.sin_port = htons(this->port);
    const int optVal = 1;
    setsockopt(this->lSocket, SOL_SOCKET, SO_REUSEADDR, (void*)&optVal, sizeof(optVal));
    if (bind(this->lSocket, (struct sockaddr *) &this->addr, sizeof(this->addr)) != 0) {
        Log::writeError("[SOCKET] Error binding to address/port:" + std::to_string(errno));
        this->closeLSocket();
        return;
    }
    if (listen(this->lSocket, QUEUE_SIZE) != 0) {
        Log::writeError("[SOCKET] Error listening:" + std::to_string(errno));
        this->closeLSocket();
        return;
    }

    // Succeeded
    Log::writeSuccess("[SOCKET] Listening socket created successfully!");
}

void Socket::closeLSocket() {
    if (this->lSocket >= 0) {
        close(this->lSocket);
        this->lSocket = -1;
    }
}

void Socket::createTSocket() {
    // Wait for a client to connect
    Log::writeInfo("[SOCKET] Waiting for a connection...");
    bool connected = false;
    while (!connected) {
        socklen_t sz = sizeof(this->addr);
        this->tSocket = accept(this->lSocket, (struct sockaddr *) &this->addr, (socklen_t *) &sz);
        if (this->tSocket < 0) {
            // Errno is set to 113 when waking from sleep
            if (errno == 113) {
                Log::writeWarning("[SOCKET] No route to host - normal behaviour after waking from sleep");
                this->closeLSocket();
                this->createLSocket();

            // Otherwise log errno
            } else {
                Log::writeError("[SOCKET] Error occurred accepting a connection: " + std::to_string(errno));
            }

        } else {
            connected = true;
            Log::writeSuccess("[SOCKET] Transfer socket connected!");
        }
    }
}

void Socket::closeTSocket() {
    if (this->tSocket >= 0) {
        close(this->tSocket);
        this->tSocket = -1;
    }
}

bool Socket::ready() {
    return (this->lSocket >= 0);
}

std::string Socket::readMessage() {
    // Simply return buffered message if there is one
    if (!this->msgBuffer.empty()) {
        std::string str = this->msgBuffer.front();
        str.shrink_to_fit();
        this->msgBuffer.pop();
        return str;
    }

    // Ensure we have a socket
    if (this->tSocket < 0) {
        this->createTSocket();
    }

    // Read until entire message found
    std::vector<char> buffer;
    do {
        char buf[READ_BYTES];
        int rd = read(this->tSocket, buf, READ_BYTES);
        if (rd < 0) {
            Log::writeError("[SOCKET] Error occurred reading from socket: " + std::to_string(errno));
            buffer.clear();

        // Reconnect on connection loss
        } else if (rd == 0) {
            Log::writeWarning("[SOCKET] Lost connection while waiting to read");
            this->closeTSocket();
            this->createTSocket();
            buffer.clear();

        } else {
            for (int i = 0; i < rd; i++) {
                buffer.push_back(buf[i]);
            }
        }
    } while (buffer.size() == 0 || buffer[buffer.size() - 1] != '\0');

    // Break up if multiple messages were received
    size_t s = 0;
    for (size_t c = 0; c < buffer.size(); c++) {
        // Once the end is found add to buffer
        if (buffer[c] == '\0') {
            std::string str(buffer.begin() + s, buffer.begin() + c);
            str.shrink_to_fit();
            this->msgBuffer.push(str);
            s = c + 1;
        }
    }

    // Return a message in the cache (there will be one at this point!)
    std::string str;
    if (!this->msgBuffer.empty()) {
        str = this->msgBuffer.front();
        str.shrink_to_fit();
        this->msgBuffer.pop();
    } else {
        str = "";
    }
    return str;
}

bool Socket::writeMessage(const std::string & str) {
    // Create message
    const char * tmp = str.c_str();
    int len = strlen(tmp) + 1;

    // Write data
    if (write(this->tSocket, tmp, len) != len) {
        Log::writeError("[SOCKET] Error writing data!");
        return false;
    }

    Log::writeInfo("[SOCKET] Wrote data '" + str + "'");
    return true;
}

Socket::~Socket() {
    this->closeTSocket();
    this->closeLSocket();
}