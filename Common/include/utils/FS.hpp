#ifndef UTILS_FS_HPP
#define UTILS_FS_HPP

#include <string>

// Helper functions for interacting with the filesystem
namespace Utils::Fs {
    // Create directories forming path
    // Returns false on an error
    bool createPath(const std::string &);

    // Copy file from src to dest
    // Returns true if successful, false otherwise
    bool copyFile(const std::string &, const std::string &);

    // All return true based on condition
    bool fileExists(const std::string &);
    bool fileReadable(const std::string &);
    bool fileWritable(const std::string &);
};

#endif