#ifndef UTILS_NX_HPP
#define UTILS_NX_HPP

#include <string>

namespace Utils::NX {
    // Start required Switch services
    void startServices();

    // Stop any started services
    void stopServices();

    // Keyboard options
    struct Keyboard {
        std::string buffer;     // Keyboard buffer (read on creation and filled on exit)
        size_t maxLength;       // Maximum input length (limited to 32 if showLine is true)
        std::string ok;         // Text to show on OK button
        bool showLine;          // Set true to show line instead of box

        // The following are only used if showLine is true
        std::string heading;
        std::string subHeading;

        // The following are only used if showLine is false
        std::string hint;
    };

    // Create switch keyboard with provided parameters
    // Returns true if successful, false otherwise
    bool getUserInput(Keyboard &);

    // Enable/disable CPU boost
    // Does nothing if state matches
    void setCPUBoost(bool);

    // Enable/disable 'media playing' flag
    // Does nothing if state matches
    void setPlayingMedia(bool);

    // Launch the program with the given program id
    // Returns true on success, false otherwise
    bool launchProgram(unsigned long long);

    // Terminate the program with the given program id
    // Returns true on success, false otherwise
    bool terminateProgram(unsigned long long);
};

#endif