#ifndef UTILS_FLAC_HPP
#define UTILS_FLAC_HPP

#include "Types.hpp"
#include <vector>

namespace Utils::FLAC {
    // Reads image from file using TagLib and returns a vector containing the image
    // Vector will be empty if no art is found
    std::vector<unsigned char> getArt(std::string);

    // Reads metadata from file using TagLib
    // ID is -1 on success (filled), -2 on success (song missing some tags), -3 on failure
    Metadata::Song getInfo(std::string);
};

#endif