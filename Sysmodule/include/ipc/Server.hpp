#ifndef IPC_SERVER_HPP
#define IPC_SERVER_HPP

#include "ipc/Types.hpp"
#include <string>
#include <vector>

// This was heavily inspired by sys-clk's ipc server, a big thanks to those
// who wrote the original C version:
// --------------------------------------------------------------------------
// "THE BEER-WARE LICENSE" (Revision 42):
// <p-sam@d3vs.net>, <natinusala@gmail.com>, <m4x@m4xw.net>
// wrote this file. As long as you retain this notice you can do whatever you
// want with this stuff. If you meet any of us some day, and you think this
// stuff is worth it, you can buy us a beer in return.  - The sys-clk authors
// --------------------------------------------------------------------------
namespace Ipc {
    class Server {
        private:
            SmServiceName serverName;       // Name of IPC server
            bool exit_;                     // Set true to exit process loop
            Handler handler;                // Function to handle request

            std::vector<Handle> handles;    // Server (index 0) and client's handles
            size_t maxHandles;              // Maximum number of clients

            // Handle requests + responses
            void parseRequest(Request &);
            void prepareResponse(uint32_t, std::vector<uint8_t> &);

            // Process a session
            bool processSession(const int32_t);
            bool processNewSession();

        public:
            // Constructor inits server (accepts name and max connection count)
            Server(const std::string &, const size_t);

            // Return the maximum number of bytes supported in one message
            size_t maxMessageSize();

            // Set the request handler function
            void setRequestHandler(Handler);

            // Signal to stop handling requests
            void exit();

            // Process any received requests (returns once exit() called)
            void process();

            // Clean up and stop the server
            ~Server();
    };
};

#endif