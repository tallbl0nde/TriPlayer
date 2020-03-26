#include <arpa/inet.h>
#include "Commands.h"
#include <cstring>
#include <netinet/in.h>
#include "Socket.hpp"
#include <sys/socket.h>
#include <sys/time.h>
#include "Utils.hpp"

// Read buffer grow size (in bytes)
#define BUFFER_SIZE 1000
// Timeout for read operations (in seconds)
#define TIMEOUT 5

namespace Utils::Socket {
    SockFD createSocket(int port) {
        // Create file descriptor
        SockFD sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            Utils::writeStdout("[SOCKET] [createSocket()] Error creating socket file descriptor!");
            return -1;
        }

        // Set address/port to connect to
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(port);

        // Actually attempt connection
        if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
            Utils::writeStdout("[SOCKET] [createSocket()] Error connecting to port " + std::to_string(port));
            closeSocket(sock);
            return -2;
        }

        // Set read timeout
        struct timeval time;
        time.tv_sec = TIMEOUT;
        time.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)& time, sizeof(time));

        // No errors! :)
        Utils::writeStdout("[SOCKET] [createSocket()] Connected successfully");
        return sock;
    }

    bool writeToSocket(SockFD sock, std::string str) {
        // Create message
        const char * tmp = str.c_str();
        int len = strlen(tmp) + 1;

        // Write data
        if (write(sock, tmp, len) != len) {
            Utils::writeStdout("[SOCKET] [writeToSocket()] Error writing data!");
            return false;
        }

        Utils::writeStdout("[SOCKET] [writeToSocket()] Wrote data '" + str + "'");
        return true;
    }

    std::string readFromSocket(SockFD sock) {
        char buf[BUFFER_SIZE] = {0};

        // Attempt to read
        if (read(sock, buf, BUFFER_SIZE) <= 0) {
            Utils::writeStdout("[SOCKET] [readFromSocket()] Error occurred reading from socket");
            return "";
        }

        // Return read chars
        std::string str(buf);
        Utils::writeStdout("[SOCKET] [readFromSocket()] Read data '" + str + "'");
        return str;
    }

    bool closeSocket(SockFD sock) {
        // Close the socket
        if (close(sock) != 0) {
            switch (errno) {
                case EBADF:
                    Utils::writeStdout("[SOCKET] [closeSocket()] Unable to close socket - invalid file descriptor");
                    break;

                default:
                    Utils::writeStdout("[SOCKET] [closeSocket()] Unable to close socket - an unknown error occurred");
                    break;
            }
            return false;
        }

        Utils::writeStdout("[SOCKET] [closeSocket()] Socket closed successfully!");
        return true;
    }
};