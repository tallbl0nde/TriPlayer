#ifndef NX_NX_HPP
#define NX_NX_HPP

#include <functional>
#include "utils/nx/Button.hpp"

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

    namespace Hid {
        // Check if the provided combo is currently pressed (order irrelevant)
        bool comboPressed(const std::vector<Button> &);
    };

    namespace Psc {
        // Prepare psc for use
        bool prepare();
        // Cleanup psc after use
        void cleanup();

        // Set callback when entering sleep
        void setSleepFunc(const std::function<void()> &);
        // Set callback when waking from sleep
        void setWakeFunc(const std::function<void()> &);

        // Monitor for power changes, and call set callbacks on appropriate state changes
        // Passed number of milliseconds to wait for event before returning
        void monitor(const size_t);
    };

    namespace Thread {
        // Start a new thread with the given function and argument
        // Uses given id to identify a thread
        // Params: ID, function, argument, stack size (use default if zero passed)
        bool create(const std::string &, void (*)(void *), void *, const size_t = 0);

        // Wait for the given thread to exit and return
        // Uses given id to identify thread
        void join(const std::string &);

        // Sleep the calling thread
        void sleepMilli(const size_t);
        void sleepNano(const size_t);
    }
};

#endif