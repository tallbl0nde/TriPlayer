#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>
#include "utils/FS.hpp"

namespace Utils::Fs {
    bool createPath(const std::string & path) {
        if (fileExists(path)) {
            return true;
        }

        return std::filesystem::create_directories(path);
    }

    bool copyFile(const std::string & src, const std::string & dst) {
        std::ifstream srcF(src, std::ios::in | std::ios::binary);
        std::ofstream destF(dst, std::ios::out | std::ios::binary);
        destF << srcF.rdbuf();
        destF.flush();
        return true;
    }

    bool fileAccessible(const std::string & file) {
        // Use stat to determine if the file can be accessed
        struct stat st;
        return (stat(file.c_str(), &st) == 0);
    }

    bool fileExists(const std::string & file) {
        return std::filesystem::exists(file);
    }

    std::vector< std::pair<std::string, bool> > getDirectoryContents(const std::string & path) {
        std::vector< std::pair<std::string, bool> > items;
        for (auto & p: std::filesystem::directory_iterator(path)) {
            items.push_back(std::make_pair(p.path().filename(), p.is_directory()));
        }
        return items;
    }

    std::string getExtension(const std::string & path) {
        return std::filesystem::path(path).extension();
    }

    std::string getParentDirectory(const std::string & path) {
        return std::filesystem::path(path).parent_path();
    }

    bool appendFile(const std::string & path, const std::vector<unsigned char> & data) {
        if (data.size() == 0) {
            return true;
        }

        // Open file
        std::FILE * fp = std::fopen(path.c_str(), "ab");
        if (fp == nullptr) {
            return false;
        }

        // Append data to file
        bool b = (std::fwrite(&data[0], sizeof(unsigned char), data.size(), fp) == (data.size() * sizeof(unsigned char)));
        std::fclose(fp);
        return b;
    }

    void deleteFile(const std::string & path) {
        std::filesystem::remove(path);
    }

    bool readFile(const std::string & path, std::vector<unsigned char> & buffer) {
        // Open file
        std::FILE * fp = std::fopen(path.c_str(), "rb");
        if (fp == nullptr) {
            return false;
        }

        // Get file size
        std::fseek(fp, 0, SEEK_END);
        size_t bytes = std::ftell(fp);
        std::fseek(fp, 0, SEEK_SET);

        // Don't read files larger than 50MB
        if (bytes > 50 * 1024 * 1024) {
            return false;
        }

        // Increase buffer size and copy
        buffer.resize(bytes);
        std::fread(&buffer[0], sizeof(unsigned char), bytes, fp);
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