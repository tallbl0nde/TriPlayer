#include <arpa/inet.h>
#include <netinet/in.h>
#include "Socket.hpp"
#include <sys/socket.h>
#include "Utils.hpp"

// Maximum number of characters to read/write
#define BUFFER 255

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

        // No errors! :)
        Utils::writeStdout("[SOCKET] [createSocket()] Connected successfully");
        return sock;
    }

    bool writeToSocket(SockFD sock, std::string str) {
        // Write data
        const char * tmp = str.c_str();
        if (write(sock, tmp, str.length()) != (int)str.length()) {
            Utils::writeStdout("[SOCKET] [writeToSocket()] Error writing data!");
            return false;
        }

        Utils::writeStdout("[SOCKET] [writeToSocket()] Wrote data '" + str + "'");
        return true;
    }

    std::string readFromSocket(SockFD sock) {
        // Accept a connection (will block!!)
        char buf[BUFFER] = {0};
        socklen_t sz = sizeof(struct sockaddr_in);
        int tmpfd = accept(sock, (struct sockaddr *) &buf, &sz);

        // Attempt to read
        if (read(tmpfd, buf, BUFFER) < 0) {
            Utils::writeStdout("[SOCKET] [readFromSocket()] Error occurred reading from socket");
            closeSocket(tmpfd);
            return "";
        }

        // Return read chars
        std::string str(buf);
        Utils::writeStdout("[SOCKET] [readFromSocket()] Read data '" + str + "'");
        closeSocket(tmpfd);
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