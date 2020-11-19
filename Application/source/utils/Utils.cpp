#include <algorithm>
#include <cmath>
#include <ctime>
#include "utils/Utils.hpp"

namespace Utils {
    std::string formatBytes(long long bytes) {
        // Divide until smaller than 1024
        double val = bytes;
        int divs = 0;
        while (val > 1023) {
            val /= 1024;
            divs++;
        }

        // Convert to string and append units
        std::string str = truncateToDecimalPlace(std::to_string(roundToDecimalPlace(val, 1)), 1);
        switch (divs) {
            case 0:
                str += " bytes";
                break;

            case 1:
                str += " KB";
                break;

            case 2:
                str += " MB";
                break;

            case 3:
                str += " GB";
                break;

            case 4:
                str += " TB";
                break;

            default:
                str += " ??";
                break;
        }
        return str;
    }

    std::string getClockString(bool in24) {
        // Get time
        std::time_t ts = std::time(nullptr);
        std::tm * tm = std::localtime(&ts);

        // Print time into buffer
        char buf[20];
        const char * fmt = (in24 ? "%R" : "%I:%M %p");
        std::strftime(buf, 20, fmt, tm);
        return std::string(buf);
    }

    // From: https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c/12468109#12468109
    static bool seedSet = false;
    std::string randomString(size_t length) {
        // Set seed if not already set
        if (!seedSet) {
            srand(time(NULL));
            seedSet = true;
        }

        auto randchar = []() -> char {
            const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[rand() % max_index];
        };
        std::string str(length, 0);
        std::generate_n(str.begin(), length, randchar);
        return str;
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

    std::vector<std::string> splitIntoWords(const std::string & str) {
        std::vector<std::string> words;

        // Iterate over each word
        std::string word = "";
        size_t pos = 0;
        while (pos < str.length()) {
            // Append chars to word until a space is reached
            if (str[pos] != ' ') {
                word.append(1, str[pos]);
            } else {
                // Don't add empty words (i.e. due to repeated spaces)
                if (word.length() > 0) {
                    words.push_back(word);
                    word = "";
                }
            }
            pos++;
        }

        // Check we haven't missed the last word
        if (word.length() > 0) {
            words.push_back(word);
        }

        return words;
    }

    size_t tokenIndex = 1;
    std::string substituteTokens(std::string str, std::string token) {
        tokenIndex = 1;
        return std::regex_replace(str, std::regex("\\$\\[1]"), token);
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
};