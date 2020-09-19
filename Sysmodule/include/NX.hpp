#ifndef NX_HPP
#define NX_HPP

#include <string>

// Switch specific functions
namespace NX {
    // Initialize all required services
    bool startServices();
    // Close any started services
    void stopServices();

    namespace Gpio {
        // Prepare gpio for use
        bool prepare();
        // Cleanup gpio after use
        void cleanup();

        // Return whether headset was unplugged, returning false on errors
        bool headsetUnplugged();
    };

    namespace Psc {
        // Prepare psc for use
        bool prepare();
        // Cleanup psc after use
        void cleanup();

        // Return if the console is entering sleep mode, returning false on errors
        // Waits for the given number of milliseconds
        bool enteringSleep(const size_t);
    };

    namespace Thread {
        // Start a new thread with the given function and argument
        // Uses given id to identify a thread
        bool create(const std::string &, void (*)(void *), void *);

        // Wait for the given thread to exit and return
        // Uses given id to identify thread
        void join(const std::string &);
    }
};

#endif