#ifndef IPC_SERVER_HPP
#define IPC_SERVER_HPP

#include <functional>
#include "ipc/Request.hpp"

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
    // Typedef this long line cause it's messy
    typedef std::function<uint32_t(Request *)> Handler;

    class Server {
        private:
            SmServiceName serverName;       // Name of IPC server
            bool error_;                    // Set true when a fatal error occurs
            Handler handler;                // Function to handle request

            std::vector<Handle> handles;    // Server (index 0) and client's handles
            size_t maxHandles;              // Maximum number of clients

            // Process a session
            bool processSession(const int32_t);
            bool processNewSession();

        public:
            // Constructor inits server (accepts name and max connection count)
            Server(const std::string &, const size_t);

            // Set the request handler function
            void setRequestHandler(Handler);

            // Process any received requests (returns false once a fatal error occurs)
            bool process();

            // Clean up and stop the server
            ~Server();
    };
};

#endif