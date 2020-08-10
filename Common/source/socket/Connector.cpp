#include <arpa/inet.h>
#include <errno.h>
#include "Log.hpp"
#include "socket/Connector.hpp"
#include <sys/socket.h>
#include <sys/time.h>

namespace Socket {
    Connector::Connector(int p) {
        this->port = p;
        this->timeout = 0;
    }

    void Connector::setTimeout(size_t secs) {
        this->timeout = secs;
    }

    Transfer * Connector::getTransferSocket() {
        // Create a socket first
        int transferSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (transferSocket < 0) {
            Log::writeError("[SOCKET] Couldn't create socket: " + std::to_string(errno));
            return nullptr;
        }

        // Set address + port to connect to
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = htons(this->port);

        // Attempt to connect
        if (connect(transferSocket, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
            Log::writeError("[SOCKET] Couldn't establish a connection: " + std::to_string(errno));
            close(transferSocket);
            return nullptr;
        }

        // Set read timeout if need be
        if (this->timeout > 0) {
            struct timeval time;
            time.tv_sec = this->timeout;
            time.tv_usec = 0;
            setsockopt(transferSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&time, sizeof(time));
        }

        // Create and return object
        return new Transfer(transferSocket);
    }
};