#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <arpa/inet.h>
#include <queue>
#include <string>

class Socket {
    private:
        // Each object has it's own struct
        struct sockaddr_in addr;
        // Port to listen on
        int port;

        // Socket to listen for connections
        int lSocket;
        // Socket to transfer data on
        int tSocket;

        // Initialize/close sockets
        void createLSocket();
        void closeLSocket();
        void createTSocket();
        void closeTSocket();

        // Buffer of messages
        std::queue<std::string> msgBuffer;

    public:
        // Constructor sets up listening socket (accepts port to listen on)
        Socket(int);

        // Call to check if initialized successfully
        bool ready();

        // Returns a message (this will block until a message is received!)
        // Also manages transfer socket behind the scenes
        std::string readMessage();

        // Send a message (returns true if successful)
        bool writeMessage(const std::string &);

        // Closes any open sockets
        ~Socket();
};

#endif