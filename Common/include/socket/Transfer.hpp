#ifndef SOCKET_TRANSFER_HPP
#define SOCKET_TRANSFER_HPP

#include <queue>
#include <string>

// Class representing a socket that is used for transferring messages
namespace Socket {
    class Transfer {
        private:
            // Is this socket connected?
            bool connected;

            // Number of read failures
            size_t failures;

            // Buffer of received messages
            std::queue<std::string> msgBuffer;

            // Socket fd used for communication
            int transferSocket;

            // Get the string at the top of the buffer
            std::string getMessageFromBuffer();

        public:
            // Uses the given fd as the transfer socket
            Transfer(int);

            // Return whether this socket is still connected
            bool isConnected();

            // Block until a message is received and return it
            std::string readMessage();

            // Write a message
            bool writeMessage(const std::string &);

            // Destructor closes the socket
            ~Transfer();
    };
};

#endif