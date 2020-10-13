#include "ipc/Request.hpp"


// IPC request header structure
struct Header {
    uint64_t magic;
    union {
        uint64_t cmdId;
        uint64_t result;
    };
};

namespace Ipc {
    // Max bytes that fit in 'header'
    constexpr size_t maxReplyBytes = 0x90 - sizeof(Header);

    Request::Request() {
        // For my sanity
        this->inArgs.clear();
        this->inData.clear();
        this->outArgs.clear();
        this->outData.clear();
        this->outMeta.clear();

        // Set default attributes
        this->cmd_ = 0;
        this->result = 0;
        this->type_ = Type::Other;
    }

    Request * Request::fromTLS() {
        // Read structure from thread-local storage
        uint8_t * base = static_cast<uint8_t *>(armGetTls());
        HipcParsedRequest hipc = hipcParseRequest(base);

        // Create object and set data/type
        Request * req = new Request();
        if (hipc.meta.type == CmifCommandType_Request) {
            req->type_ = Type::Request;

            // Validate header
            Header * header = static_cast<Header *>(cmifGetAlignedDataStart(hipc.data.data_words, base));
            size_t headerSize = hipc.meta.num_data_words * 4;
            if (!header || headerSize < sizeof(Header) || header->magic != CMIF_IN_HEADER_MAGIC) {
                // Delete object and return nullptr as a bad header is an error
                delete req;
                return nullptr;
            }

            // We appear to have a valid request, so copy relevant data
            req->cmd_ = header->cmdId;
            if (headerSize > sizeof(Header)) {
                uint8_t * ptr = reinterpret_cast<uint8_t *>(header) + sizeof(Header);
                size_t size = headerSize - sizeof(Header);
                req->inArgs = std::vector<uint8_t>(ptr, ptr + size);
            }

        } else if (hipc.meta.type == CmifCommandType_Close) {
            req->type_ = Type::Close;

        } else {
            req->type_ = Type::Other;
        }

        // Copy received data if there is some
        if (hipc.meta.num_send_buffers > 0) {
            uint8_t * ptr = static_cast<uint8_t *>(hipcGetBufferAddress(hipc.data.send_buffers));
            size_t size = hipcGetBufferSize(hipc.data.send_buffers);
            req->inData = std::vector<uint8_t>(ptr, ptr + size);
        }

        // Copy metadata about receiving buffers (avoid issues due to TLS overwrites)
        for (size_t i = 0; i < hipc.meta.num_recv_buffers; i++) {
            req->outMeta.push_back(hipc.data.recv_buffers[i]);
        }

        return req;
    }

    void Request::toResponseTLS() {
        // First copy any reply data if it is present
        if (!this->outData.empty() && !this->outMeta.empty()) {
            size_t size = hipcGetBufferSize(&this->outMeta[0]);
            if (this->outData.size() < size) {
                size = this->outData.size();
            }
            std::memcpy(hipcGetBufferAddress(&this->outMeta[0]), &this->outData[0], size);
        }

        // Create response on thread-local storage
        uint8_t * base = static_cast<uint8_t *>(armGetTls());
        HipcRequest hipc = hipcMakeRequestInline(base,
            .type = CmifCommandType_Request,
            .num_data_words = static_cast<uint32_t>(sizeof(Header) + this->outArgs.size() + 0x10)/4,
        );

        // Create header
        Header * header = static_cast<Header *>(cmifGetAlignedDataStart(hipc.data_words, base));
        header->magic = CMIF_OUT_HEADER_MAGIC;
        header->result = this->result;

        // Append reply 'value'
        if (R_SUCCEEDED(this->result) && !this->outArgs.empty()) {
            size_t size = (maxReplyBytes < this->outArgs.size() ? maxReplyBytes : this->outArgs.size());
            std::memcpy(reinterpret_cast<uint8_t *>(header) + sizeof(Header), &this->outArgs[0], size);
        }
    }

    uint64_t Request::cmd() {
        return this->cmd_;
    }

    void Request::setResult(const uint32_t r) {
        this->result = r;
    }

    Request::Type Request::type() {
        return this->type_;
    }

    const std::vector<uint8_t> & Request::getRequestBuffer() {
        return this->inData;
    }

    const std::vector<uint8_t> & Request::getReplyBuffer() {
        return this->outData;
    }

    Result Request::appendReplyValue(const std::string & str) {
        // Append each character in string and then null terminator
        for (size_t i = 0; i < str.length(); i++) {
            this->outArgs.push_back(str[i]);
        }
        this->outArgs.push_back('\0');
        return Result::Ok;
    }

    Result Request::readRequestValue(std::string & out) {
        // Ensure we have data to convert
        if (this->inArgs.empty()) {
            return Result::BadInput;
        }

        // Read into string and remove from buffer
        out = std::string(reinterpret_cast<char *>(&this->inArgs[0]));
        this->inArgs.erase(this->inArgs.begin(), this->inArgs.begin() + out.length() + 1);
        return Result::Ok;
    }
};