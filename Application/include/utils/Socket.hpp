#ifndef UTILS_SOCKET_HPP
#define UTILS_SOCKET_HPP

#include "Types.hpp"

// A bunch of helper functions for sockets (which in this case is used for
// communicating with the sysmodule)
namespace Utils::Socket {
    // Create a socket and return SockFD (less than zero if an error occurred)
    // Accepts port number
    SockFD createSocket(int);

    // Set read timeout on socket
    void setTimeout(SockFD, int);

    // Write given string to socket (returns true on success)
    bool writeToSocket(SockFD, std::string);

    // Read from socket (returns read value as string or empty string if failed)
    std::string readFromSocket(SockFD);

    // Closes a socket (returns true on success)
    bool closeSocket(SockFD);
};

#endif