#ifndef SOCKET_HPP
#define SOCKET_HPP

// Helper functions for Socket related communication
// I should probably redo this but I'll likely move to IPC eventually
namespace Socket {
    // Create listening socket
    bool createListeningSocket();
    // Closes listening socket
    void closeListeningSocket();

    // Wait for a connection before timing out
    void acceptConnection();
    // Returns if the transfer socket is connected
    bool haveConnection();
    // Close accepted connection
    void closeConnection();

    // Attempt to read data before timing out (blank on timeout)
    std::string readData();

    // Write data to the socket
    void writeData(const std::string &);
};

#endif