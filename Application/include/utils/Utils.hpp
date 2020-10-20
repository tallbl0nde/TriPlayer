#ifndef UTILS_HPP
#define UTILS_HPP

#include <set>
#include <string>
#include <vector>

// General helper functions
namespace Utils {
    // Returns the current time as a string
    // Pass true to get in 24-hour format
    std::string getClockString(bool = false);

    // Return a random alpha-numeric string with given length
    std::string randomString(size_t);

    // Round the given double to the specified number of decimal places
    float roundToDecimalPlace(float, unsigned int);

    // Format seconds in HH:MM:SS
    std::string secondsToHMS(unsigned int);

    // Format seconds in x hours, y minutes
    std::string secondsToHoursMins(unsigned int);

    // Splits the given string into words (splits on space)
    std::vector<std::string> splitIntoWords(const std::string &);

    // Truncate string to given decimal places (don't use on strings without a decimal!)
    // Does nothing if outside of range or no decimal place
    std::string truncateToDecimalPlace(std::string, unsigned int);

    // Write to stdout if nxlink is enabled
    void writeStdout(std::string);

    // Remove duplicates from a vector without sorting
    template<typename T>
    std::vector<T> removeDuplicates(const std::vector<T> & old) {
        // Set to keep track of elements
        std::set<T> set;

        // Iterate and copy if not in set
        std::vector<T> unique;
        for (T element : old) {
            if (set.insert(element).second) {
                unique.push_back(element);
            }
        }
        return unique;
    }
};

#endif