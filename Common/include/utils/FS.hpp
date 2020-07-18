#ifndef UTILS_FS_HPP
#define UTILS_FS_HPP

#include <string>
#include <vector>

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

    // Returns contents of directory
    // First element in pair is path, second is set true if directory
    std::vector< std::pair<std::string, bool> > getDirectoryContents(const std::string &);
    // Returns parent directory
    std::string getParentDirectory(const std::string &);

    // Delete a file
    void deleteFile(const std::string &);
    // Write entire contents of buffer to file
    bool writeFile(const std::string &, const std::vector<unsigned char> &);
};

#endif