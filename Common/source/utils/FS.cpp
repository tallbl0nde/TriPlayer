#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>
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


    void deleteFile(const std::string & path) {
        std::filesystem::remove(path);
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