#ifndef UTILS_HPP
#define UTILS_HPP

#include <regex>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

// General helper functions
namespace Utils {
    // Return the given number as a formatted byte string
    // eg. 2 KB or 102.3 MB
    std::string formatBytes(long long);

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

    // Base case for below
    std::string substituteTokens(std::string, std::string);

    // Substitute any tokens with the format $[x] with the provided arguments
    extern size_t tokenIndex;
    template<typename... Args>
    std::string substituteTokens(std::string str, std::string token, Args... tokens) {
        str = std::regex_replace(str, std::regex("\\$\\[" + std::to_string(tokenIndex) + "]"), token);
        tokenIndex++;
        return substituteTokens(str, tokens...);
    }

    // Return string with all characters changed to lowercase
    std::string toLowercase(std::string);

    // Return string with all characters changed to uppercase
    std::string toUppercase(std::string);

    // Truncate string to given decimal places (don't use on strings without a decimal!)
    // Does nothing if outside of range or no decimal place
    std::string truncateToDecimalPlace(std::string, unsigned int);

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