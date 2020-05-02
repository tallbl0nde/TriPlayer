#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include "Log.hpp"
#include "Socket.hpp"
#include <sys/socket.h>
#include <sys/time.h>

// Characters to read/write in one go
#define BUFFER_SIZE 2000

namespace Utils::Socket {
    SockFD createSocket(int port) {
        // Create file descriptor
        SockFD sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            Log::writeError("[SOCKET] Error creating socket: " + std::to_string(errno));
            return -1;
        }

        // Set address/port to connect to
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(port);

        // Actually attempt connection
        if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
            Log::writeError("[SOCKET] Error connecting to sysmodule: " + std::to_string(errno));
            closeSocket(sock);
            return -2;
        }

        return sock;
    }

    void setTimeout(SockFD sock, int sec) {
        // Set read timeout
        struct timeval time;
        time.tv_sec = sec;
        time.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)& time, sizeof(time));
    }

    bool writeToSocket(SockFD sock, std::string str) {
        // Create message
        const char * tmp = str.c_str();
        int len = strlen(tmp) + 1;

        // Write data
        if (write(sock, tmp, len) != len) {
            Log::writeError("[SOCKET] An error occurred while writing data: " + std::to_string(errno));
            return false;
        }

        // Don't bother concating if it won't be logged
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("[SOCKET] Wrote data: '" + str + "'");
        }
        return true;
    }

    std::string readFromSocket(SockFD sock) {
        char buf[BUFFER_SIZE] = {0};

        // Attempt to read
        if (read(sock, buf, BUFFER_SIZE) <= 0) {
            Log::writeError("[SOCKET] Error occurred reading from socket: " + std::to_string(errno));
            return "";
        }

        // Return read chars
        std::string str(buf);

        // Don't bother concating if it won't be logged
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("[SOCKET] Read data: '" + str + "'");
        }
        return str;
    }

    bool closeSocket(SockFD sock) {
        // Close the socket
        if (close(sock) != 0) {
            Log::writeError("[SOCKET] Error occurred while closing socket: " + std::to_string(errno));
            return false;
        }

        Log::writeSuccess("[SOCKET] Socket closed successfully!");
        return true;
    }
};