#ifndef IPC_HELPERS_HPP
#define IPC_HELPERS_HPP

#include <cstddef>
#include "ipc/Result.hpp"
#include <string>
#include <vector>

// Function to help read/write to the buffer sent/received over IPC
// These assume a little endian ordering!
namespace Ipc::Helpers {
    // Append a raw type to the buffer
    template <typename T>
    Result appendValueToBuffer(std::vector<uint8_t> & buf, const T value) {
        const uint8_t * ptr = reinterpret_cast<const uint8_t *>(&value);
        for (size_t i = 0; i < sizeof(value); i++) {
            buf.push_back(ptr[i]);
        }
        return Result::Ok;
    }

    // Append a std::string to the buffer
    Result appendStringToBuffer(std::vector<uint8_t> & buf, const std::string & str) {
        for (size_t i = 0; i < str.length(); i++) {
            buf.push_back(str[i]);
        }
        buf.push_back('\0');
        return Result::Ok;
    }

    // Read a raw type from the specified position (and increment)
    template <typename T>
    Result readValueFromBuffer(std::vector<uint8_t> & buf, size_t & pos, T & out) {
        if (pos + sizeof(out) >= buf.size()) {
            return Result::BadInput;
        }

        std::memcpy(&out, &buf[pos], sizeof(out));
        pos += sizeof(out);
        return Result::Ok;
    }

    // Read a raw type from the beginning of the buffer
    template <typename T>
    Result readValueFromBuffer(std::vector<uint8_t> & buf, T & out) {
        if (sizeof(out) >= buf.size()) {
            return Result::BadInput;
        }

        std::memcpy(&out, &buf[0], sizeof(out));
        return Result::Ok;
    }

    // Read a std::string from the specified position
    Result readStringFromBuffer(std::vector<uint8_t> & buf, size_t & pos, std::string & out) {
        if (pos >= buf.size()) {
            return Result::BadInput;
        }

        out = std::string(reinterpret_cast<char *>(&buf[pos]));
        pos += out.length() + 1;
        return Result::Ok;
    }

    // Read a std::string from the beginning of the buffer
    Result readStringFromBuffer(std::vector<uint8_t> & buf, std::string & out) {
        if (buf.empty()) {
            return Result::BadInput;
        }

        out = std::string(reinterpret_cast<char *>(&buf[0]));
        return Result::Ok;
    }
};

#endif