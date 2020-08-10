#ifndef SOCKET_LISTENER_HPP
#define SOCKET_LISTENER_HPP

#include <arpa/inet.h>
#include <atomic>
#include <mutex>
#include "socket/Transfer.hpp"

// Class representing a socket that listens for connections
namespace Socket {
    class Listener {
        private:
            // Each object has it's own struct
            struct sockaddr_in addr;

            // Is the object is listening for connections?
            std::atomic<bool> listening;

            // Port to listen on
            int port;

            // Listening socket fd
            int listeningSocket;

            // Actually create the socket
            void createListeningSocket();

            // Close the socket
            void closeListeningSocket();

            // Queue of transfer sockets to return
            std::queue<Transfer *> sockets;

            // Mutex to safely handle concurrent access
            std::mutex mutex;

        public:
            // Creates the listening socket on the given port
            Listener(int);

            // Returns true if listening for connections
            bool isListening();

            // Returns true if there is a transfer socket connected
            bool hasTransferSocket();

            // Returns the created transfer socket object (nullptr if none)
            // Needs to be deleted by the caller
            Transfer * getTransferSocket();

            // This function actually processes everything and should be called in
            // it's own thread
            // Pass reference to a boolean which when set true indicates this thread to stop
            void listen(std::atomic<bool> &);

            // Destructor deletes it's transfer sockets and listening socket
            ~Listener();
    };
};

#endif