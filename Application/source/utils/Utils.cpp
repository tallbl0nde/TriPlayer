#include <cmath>
#include <filesystem>
#include <sys/stat.h>
#include "Utils.hpp"

namespace Utils {
    time_t getModifiedTimestamp(std::string path) {
        struct stat st;
        if (stat(path.c_str(), &st) == 0) {
            return st.st_mtime;
        }
        return 0;
    }

    std::vector<std::string> getFilesWithExt(std::string dir, std::string ext) {
        std::vector<std::string> paths;

        // Recursive search
        for (auto &p: std::filesystem::recursive_directory_iterator(dir)) {
            if (p.path().extension() == ext) {
                paths.push_back(p.path().string());
            }
        }

        return paths;
    }

    float roundToDecimalPlace(float val, unsigned int p) {
        for (unsigned int i = 0; i < p; i++) {
            val *= 10.0;
        }
        val = std::round(val);
        for (unsigned int i = 0; i < p; i++) {
            val /= 10.0;
        }
        return val;
    }

    std::string secondsToHMS(unsigned int sec) {
        std::string str = "";
        // Hours
        int h = sec/3600;
        if (h > 0) {
            str += std::to_string(h) + ":";
        }

        // Minutes
        int m = ((sec/60)%60);
        if (str.length() > 0 && m < 10) {
            str += "0";
        }
        str += std::to_string(m);

        // Seconds
        str += ":";
        int s = sec%60;
        if (s < 10) {
            str += "0";
        }
        str += std::to_string(s);

        return str;
    }

    std::string secondsToHoursMins(unsigned int sec) {
        std::string str = "";

        // Hours
        int h = sec/3600;
        if (h != 0) {
            str += std::to_string(h);
        }
        if (h == 1) {
            str += " hour";
        } else if (h != 0) {
            str += " hours";
        }

        // Minutes
        int m = ((sec/60)%60);
        if (h > 0) {
            str += ", ";
        }
        str += std::to_string(m);
        if (m == 1) {
            str += " minute";
        } else {
            str += " minutes";
        }

        return str;
    }

    std::string truncateToDecimalPlace(std::string str, unsigned int p) {
        size_t dec = str.find(".");
        if (dec == std::string::npos || p >= str.length() - dec) {
            return str;
        }

        // Cut off decimal place if zero
        if (p == 0) {
            dec--;
        }

        return str.substr(0, dec + 1 + p);
    }

    std::string unicodeToASCII(char * utf, unsigned int len) {
        if (len <= 1) {
            return "";
        }

        bool isLE = ((*(utf) == 0xFF && *(utf + 1) == 0xFE) ? true : false);
        unsigned int chars = (len - 2)/2;
        char * tmp = new char[chars];
        for (size_t i = (isLE ? 2 : 3); i < len; i += 2) {
            tmp[(i - 2)/2] = *(utf + i);
        }
        while (tmp[chars - 1] == '\0') {
            chars--;
        }
        std::string str(&tmp[0], chars);
        delete[] tmp;
        return str;
    }
};