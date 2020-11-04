#ifndef UTILS_MP3_HPP
#define UTILS_MP3_HPP

#include "Types.hpp"
#include <vector>

namespace Utils::MP3 {
    // Reads image(s) from ID3 tags using TagLib and returns a vector containing the image data 
    // The vector will be size 0 if no art is found
    // Pass path of file
    std::vector<unsigned char> getArtFromID3(std::string);

    // Reads ID3 tags of file using TagLib and returns SongInfo
    // ID is -1 on success (filled), -2 on success (song has no tags), -3 on failure
    // Pass path of file
    Metadata::Song getInfoFromID3(std::string);
};

#endif