#ifndef UTILS_HPP
#define UTILS_HPP

#include "Types.hpp"
#include <vector>

// General helper functions
namespace Utils {
    // Recursively scan given directory for files with given extension
    // Returns vector of paths: give directory, path
    std::vector<std::string> getFilesWithExt(std::string, std::string);

    // Reads ID3 tags of file using mpg123 and returns SongInfo
    // ID is -1 on success (filled), -2 on success (song has no tags), -3 on failure
    // Pass path of file
    SongInfo getInfoFromID3(std::string);

    // Round the given double to the specified number of decimal places
    float roundToDecimalPlace(float, unsigned int);

    // Truncate string to given decimal places (don't use on strings without a decimal!)
    // Does nothing if outside of range or no decimal place
    std::string truncateToDecimalPlace(std::string, unsigned int);

    // Write to stdout if nxlink is enabled
    void writeStdout(std::string);
};

#endif