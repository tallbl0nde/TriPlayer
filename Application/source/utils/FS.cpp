#include <cstdio>
#include <filesystem>
#include <fstream>
#include "FS.hpp"

namespace Utils::Fs {
    bool createPath(const std::string & path) {
        return std::filesystem::create_directories(path);
    }

    bool copyFile(const std::string & src, const std::string & dst) {
        std::ifstream srcF(src, std::ios::binary);
        std::ofstream destF(dst, std::ios::binary);
        destF << srcF.rdbuf();
        destF.flush();
        return true;
    }

    bool fileExists(const std::string & file) {
        return std::filesystem::exists(file);
    }

    bool fileReadable(const std::string & file) {
        std::FILE * fp = std::fopen(file.c_str(), "r+");
        if (fp == NULL) {
            errno = 0;
            return false;
        }
        std::fclose(fp);
        return true;
    }

    bool fileWritable(const std::string & file) {
        // Check that the file exists first
        if (!fileExists(file)) {
            return false;
        }

        // Now check if writable
        std::FILE * fp = std::fopen(file.c_str(), "w+");
        if (fp == NULL) {
            errno = 0;
            return false;
        }
        std::fclose(fp);
        return true;
    }
};