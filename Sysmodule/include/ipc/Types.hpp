#ifndef IPC_TYPES_HPP
#define IPC_TYPES_HPP

#include <functional>
#include <switch.h>

namespace Ipc {
    // Message data
    struct Data {
        uint64_t cmdId;
        void * ptr;
        size_t size;
    };

    // Message header
    struct Header {
        uint64_t magic;
        union {
            uint64_t cmdId;
            uint64_t result;
        };
    };

    // IPC Request
    struct Request {
        HipcParsedRequest hipc;
        Data data;
    };

    // Handler function structure
    typedef std::function<uint32_t(Request &, std::vector<uint8_t> &)> Handler;
};

#endif