#ifndef UTILS_BUFFER_HPP
#define UTILS_BUFFER_HPP

#include <string>
#include <vector>

// Helpers to read/write values to a buffer (vector)
namespace Utils::Buffer {
    // Append a string to a biffer
    bool appendString(std::vector<uint8_t> &, const std::string &);

    // Append value onto a buffer
    template <typename T>
    bool appendValue(std::vector<uint8_t> & buf, const T val) {
        const uint8_t * ptr = reinterpret_cast<const uint8_t *>(&val);
        for (size_t i = 0; i < sizeof(val); i++) {
            buf.push_back(ptr[i]);
        }
        return true;
    }

    // Retrieve a string from buffer and increment position (returns false if outside of buffer)
    bool readString(const std::vector<uint8_t> &, size_t &, std::string &);

    // Retrieve value from buffer and increment position (returns false if outside of buffer)
    template <typename T>
    bool readValue(const std::vector<uint8_t> & buf, size_t & pos, T & val) {
        // Check we have enough bytes to read
        size_t bytes = sizeof(val);
        if (pos + bytes > buf.size()) {
            return false;
        }

        // Read required number of bytes and move position
        std::memcpy(&val, &buf[pos], bytes);
        pos += bytes;
        return true;
    }
};

#endif