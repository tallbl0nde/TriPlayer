#ifndef UTILS_NX_HPP
#define UTILS_NX_HPP

#include <string>
#include <switch.h>

namespace Utils::NX {
    // Create switch keyboard with provided parameters
    // max string length, ok text, heading, subheading, hint (optional), initial string (optional)
    std::string getUserInput(unsigned int, std::string, std::string, std::string, std::string = "", std::string = "");
};

#endif