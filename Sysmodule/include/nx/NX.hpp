#ifndef NX_NX_HPP
#define NX_NX_HPP

#include <string>
#include <vector>

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
        // Valid buttons which can form a key combination
        enum class Button {
            A = 0,
            B = 1,
            X = 2,
            Y = 3,
            LSTICK = 4,
            RSTICK = 5,
            L = 6,
            R = 7,
            ZL = 8,
            ZR = 9,
            PLUS = 10,
            MINUS = 11,
            DLEFT = 12,
            DUP = 13,
            DRIGHT = 14,
            DDOWN = 15
        };

        // Convert a string to a key combination (empty if any key is invalid)
        // Valid delimiters are ' ' and '+' and is not case sensitive
        std::vector<Button> stringToCombo(const std::string &);

        // Check if the provided combo is currently pressed (order irrelevant)
        bool comboPressed(const std::vector<Button> &);
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