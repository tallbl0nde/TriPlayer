#ifndef SOCKET_CONNECTOR_HPP
#define SOCKET_CONNECTOR_HPP

#include "socket/Transfer.hpp"

// Class which will attempt to connect to a socket and return
// a Transfer socket if the connection succeeded
namespace Socket {
    class Connector {
        private:
            // Port to connect to
            int port;

            // Read timeout in seconds
            size_t timeout;

        public:
            // The constructor doesn't actually do much except for set the port
            Connector(int);

            // Set an optional read timeout (in seconds)
            // This only effects future connections
            void setTimeout(size_t);

            // Attempt to connect to the socket
            // Returns a Transfer socket object on success, nullptr on an error
            Transfer * getTransferSocket();
    };
};

#endif