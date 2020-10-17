#ifndef IPC_REQUEST_HPP
#define IPC_REQUEST_HPP

#include <cstring>
#include "ipc/Result.hpp"
#include <string>
#include <switch.h>
#include <vector>
#include "utils/Buffer.hpp"

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
            size_t inArgsPos;                           // Position to read from next
            std::vector<uint8_t> inData;                // Received data
            size_t inDataPos;                           // Position to read from next

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

            // Append a value to reply buffer
            template <typename T>
            Result appendReplyData(const T value) {
                return (Utils::Buffer::appendValue(this->outData, value) ? Result::Ok : Result::BadInput);
            }

            // Append a string to reply buffer
            Result appendReplyData(const std::string & str) {
                return (Utils::Buffer::appendString(this->outData, str) ? Result::Ok : Result::BadInput);
            }

            // Append value to reply 'value'
            template <typename T>
            Result appendReplyValue(const T value) {
                return (Utils::Buffer::appendValue(this->outArgs, value) ? Result::Ok : Result::BadInput);
            }

            // Append a string to reply 'value'
            Result appendReplyValue(const std::string & str) {
                return (Utils::Buffer::appendString(this->outArgs, str) ? Result::Ok : Result::BadInput);
            }

            // Sequentially read from received data
            template <typename T>
            Result readRequestData(T & out) {
                return (Utils::Buffer::readValue(this->inData, this->inDataPos, out) ? Result::Ok : Result::BadInput);
            }

            // Sequentially read a string from received data
            Result readRequestData(std::string & out) {
                return (Utils::Buffer::readString(this->inData, this->inDataPos, out) ? Result::Ok : Result::BadInput);
            }

            // Sequentially read from received 'arguments'
            template <typename T>
            Result readRequestValue(T & out) {
                return (Utils::Buffer::readValue(this->inArgs, this->inArgsPos, out) ? Result::Ok : Result::BadInput);
            }

            // Sequentially read a string from received 'arguments'
            Result readRequestValue(std::string & out) {
                return (Utils::Buffer::readString(this->inArgs, this->inArgsPos, out) ? Result::Ok : Result::BadInput);
            }

            // Destructor frees allocated memory
            ~Request();
    };
};

#endif