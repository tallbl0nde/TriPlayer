#ifndef UTILS_NX_HPP
#define UTILS_NX_HPP

#include <string>

namespace Utils::NX {
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
};

#endif