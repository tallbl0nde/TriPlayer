#ifndef UTILS_MP3_HPP
#define UTILS_MP3_HPP

#include "Types.hpp"

namespace Utils::MP3 {
    // Reads ID3 tags of file using mpg123 and returns SongInfo
    // ID is -1 on success (filled), -2 on success (song has no tags), -3 on failure
    // Pass path of file
    SongInfo getInfoFromID3(std::string);
};

#endif