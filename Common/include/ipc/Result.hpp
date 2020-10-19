#ifndef IPC_RESULT_HPP
#define IPC_RESULT_HPP

// These are all possible 'result' values returned by any aspect of the IPC system
namespace Ipc {
    // Possible results
    enum class Result {
        Ok,                 // Everything excuted as expected
        BadInput,           // Input was not what was expected
        SubQueueFull,       // The sysmodule's subqueue is full
        Unknown             // An unexpected error occurred
    };
};

#endif