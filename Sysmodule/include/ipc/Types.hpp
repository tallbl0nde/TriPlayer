#ifndef IPC_TYPES_HPP
#define IPC_TYPES_HPP

#include <functional>
#include <switch.h>
#include <vector>

namespace Ipc {
    // Message data (command + arguments)
    struct Data {
        uint64_t cmdId;
        std::vector<uint8_t> args;
    };

    // Message header
    struct Header {
        uint64_t magic;
        union {
            uint64_t cmdId;
            uint64_t result;
        };
    };

    // IPC Request object
    struct Request {
        HipcParsedRequest hipc;             // Internal usage only!
        Data params;                        // See Data struct
        std::vector<uint8_t> inData;        // Input buffer
        std::vector<uint8_t> outData;       // Output buffer
    };

    // Handler function structure
    typedef std::function<uint32_t(Request &)> Handler;
};

#endif