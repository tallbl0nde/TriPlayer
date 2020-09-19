#ifndef NX_HPP
#define NX_HPP

// Switch specific functions
namespace NX {
    // Initialize GPIO service
    bool initializeGpio();
    // Stop GPIO service
    void exitGpio();

    // Return whether headset was unplugged, returning false on errors
    bool gpioHeadsetUnplugged();

    // Initialize PSC service
    bool initializePsc();
    // Stop PSC service
    void exitPsc();

    // Return if the console is entering sleep mode, returning false on errors
    // Waits for the given number of milliseconds
    bool pscEnteringSleep(const size_t);
};

#endif