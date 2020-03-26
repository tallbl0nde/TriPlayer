#ifndef UTILS_HPP
#define UTILS_HPP

#include <ctime>
#include <string>
#include <vector>

// General helper functions
namespace Utils {
    // Returns the date/time (as time_t) the given file was modified
    // (returns 0 on an error)
    time_t getModifiedTimestamp(std::string);

    // Recursively scan given directory for files with given extension
    // Returns vector of paths: give directory, path
    std::vector<std::string> getFilesWithExt(std::string, std::string);

    // Round the given double to the specified number of decimal places
    float roundToDecimalPlace(float, unsigned int);

    // Format seconds in HH:MM:SS
    std::string secondsToHMS(unsigned int);

    // Truncate string to given decimal places (don't use on strings without a decimal!)
    // Does nothing if outside of range or no decimal place
    std::string truncateToDecimalPlace(std::string, unsigned int);

    // 'Converts' UTF-16 to ASCII by dropping the other byte
    // Takes pointer to first char (should be BOM), number of bytes to read
    std::string unicodeToASCII(char *, unsigned int);

    // Write to stdout if nxlink is enabled
    void writeStdout(std::string);
};

#endif