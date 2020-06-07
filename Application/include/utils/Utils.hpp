#ifndef UTILS_HPP
#define UTILS_HPP

#include <atomic>
#include <ctime>
#include "Database.hpp"
#include <string>
#include <vector>

// Stages of update process (used to update UI)
enum class ProcessStage {
    Search,     // Search music folder for paths
    Parse,      // Parse each file's metadata
    Update,     // Update/clean database
    Done
};

// General helper functions
namespace Utils {
    // Returns the date/time (as time_t) the given file was modified
    // (returns 0 on an error)
    time_t getModifiedTimestamp(std::string);

    // Recursively scan given directory for files with given extension
    // Returns vector of paths: give directory, path
    std::vector<std::string> getFilesWithExt(std::string, std::string);

    // Updates the provided database to reflect the state of files on the sd card in /music
    // Atomics are used to provide the current status
    void processFileChanges(Database *, std::atomic<int> &, std::atomic<ProcessStage> &, std::atomic<int> &);

    // Round the given double to the specified number of decimal places
    float roundToDecimalPlace(float, unsigned int);

    // Format seconds in HH:MM:SS
    std::string secondsToHMS(unsigned int);

    // Format seconds in x hours, y minutes
    std::string secondsToHoursMins(unsigned int);

    // Truncate string to given decimal places (don't use on strings without a decimal!)
    // Does nothing if outside of range or no decimal place
    std::string truncateToDecimalPlace(std::string, unsigned int);

    // Write to stdout if nxlink is enabled
    void writeStdout(std::string);
};

#endif