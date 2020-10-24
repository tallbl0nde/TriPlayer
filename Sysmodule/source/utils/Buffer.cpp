#include <cstring>
#include "utils/Buffer.hpp"

namespace Utils::Buffer {
    bool appendString(std::vector<uint8_t> & buf, const std::string & str) {
        // Iterate over string
        for (size_t i = 0; i < str.size(); i++) {
            buf.push_back(str[i]);
        }
        buf.push_back('\0');
        return true;
    }

    bool readString(const std::vector<uint8_t> & buf, size_t & pos, std::string & str) {
        if (pos >= buf.size()) {
            return false;
        }

        str = std::string(reinterpret_cast<const char *>(&buf[pos]));
        pos += str.length() + 1;
        return true;
    }
};