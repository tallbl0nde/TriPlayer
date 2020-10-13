#ifndef IPC_REQUEST_HPP
#define IPC_REQUEST_HPP

#include <cstring>
#include "ipc/Result.hpp"
#include <string>
#include <switch.h>
#include <vector>

// The Request class encapsulates all data/functionality related to an IPC request.
// All data is copied from the thread-local storage which should allow other IPC calls
// to be made in between operations on instances of this class.
namespace Ipc {
    class Request {
        public:
            // Request type
            enum class Type {
                Request,        // Standard request for data
                Close,          // Close connection
                Other           // Other, unhandled type
            };

        private:
            uint64_t cmd_;                              // IPC command id
            uint32_t result;                            // IPC result code
            Type type_;                                 // Request type (see enum)

            std::vector<uint8_t> inArgs;                // Received 'arguments'
            std::vector<uint8_t> inData;                // Received data

            std::vector<uint8_t> outArgs;               // Reply value(s)
            std::vector<uint8_t> outData;               // Reply data
            std::vector<HipcBufferDescriptor> outMeta;  // Copy of hipc metadata

            // Private constructor as we can instantiate a request using different data
            Request();

        public:
            // Create a request from the thread-local storage
            // Returns nullptr on a fatal error
            static Request * fromTLS();

            // Use class members to construct a response on thread-local storage
            void toResponseTLS();

            // Return command id
            uint64_t cmd();

            // Set result code to return to caller
            void setResult(const uint32_t);

            // Return type of request
            Type type();

            // Return a reference to the received data buffer (as a vector)
            const std::vector<uint8_t> & getRequestBuffer();

            // Return a reference to the reply data buffer (as a vector)
            const std::vector<uint8_t> & getReplyBuffer();

            // Append value to reply 'value'
            template <typename T>
            Result appendReplyValue(const T value) {
                const uint8_t * ptr = reinterpret_cast<const uint8_t *>(&value);
                for (size_t i = 0; i < sizeof(value); i++) {
                    this->outArgs.push_back(ptr[i]);
                }
                return Result::Ok;
            }

            // Append a string to reply 'value'
            Result appendReplyValue(const std::string &);

            // Sequentially read from received 'arguments'
            template <typename T>
            Result readRequestValue(T & out) {
                // Check we have enough bytes
                size_t bytes = sizeof(out);
                if (this->inArgs.size() < bytes) {
                    return Result::BadInput;
                }

                // Read required number of bytes and discard
                std::memcpy(&out, &this->inArgs[0], bytes);
                this->inArgs.erase(this->inArgs.begin(), this->inArgs.begin() + bytes);
                return Result::Ok;
            }

            // Sequentially read a string from received 'arguments'
            Result readRequestValue(std::string &);

            // Destructor frees allocated memory
            ~Request();
    };
};

#endif