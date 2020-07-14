#include <cstdio>
#include <filesystem>
#include <fstream>
#include "utils/FS.hpp"

namespace Utils::Fs {
    bool createPath(const std::string & path) {
        return std::filesystem::create_directories(path);
    }

    bool copyFile(const std::string & src, const std::string & dst) {
        std::ifstream srcF(src, std::ios::in | std::ios::binary);
        std::ofstream destF(dst, std::ios::out | std::ios::binary);
        destF << srcF.rdbuf();
        destF.flush();
        return true;
    }

    bool fileExists(const std::string & file) {
        return std::filesystem::exists(file);
    }

    // Fix these functions eventually
    bool fileReadable(const std::string & file) {
        std::FILE * fp = std::fopen(file.c_str(), "rb");
        if (fp == NULL) {
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
        std::FILE * fp = std::fopen(file.c_str(), "a+b");
        if (fp == NULL) {
            return false;
        }
        std::fclose(fp);
        return true;
    }

    bool writeFile(const std::string & path, const std::vector<unsigned char> & data) {
        std::FILE * fp = std::fopen(path.c_str(), "wb");
        if (fp == NULL) {
            return false;
        }
        bool b = (std::fwrite(&data[0], sizeof(unsigned char), data.size(), fp) == data.size()*sizeof(unsigned char));
        std::fclose(fp);
        return b;
    }
};