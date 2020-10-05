#include <cstring>
#include "ipc/Server.hpp"
#include "Log.hpp"

// This was heavily inspired by sys-clk's ipc server, a big thanks to those
// who wrote the original C version:
// --------------------------------------------------------------------------
// "THE BEER-WARE LICENSE" (Revision 42):
// <p-sam@d3vs.net>, <natinusala@gmail.com>, <m4x@m4xw.net>
// wrote this file. As long as you retain this notice you can do whatever you
// want with this stuff. If you meet any of us some day, and you think this
// stuff is worth it, you can buy us a beer in return.  - The sys-clk authors
// --------------------------------------------------------------------------
namespace Ipc {
    constexpr size_t maxReplyBytes = 0x90 - sizeof(Header);     // Max bytes that fit in 'header'
    constexpr uint64_t waitTimeout = UINT64_MAX;                // Wait timeout when processing

    Server::Server(const std::string & name, const size_t maxClients) {
        // Set status variables
        this->error_ = false;
        this->handler = nullptr;
        this->maxHandles = maxClients + 1;
        this->handles.reserve(this->maxHandles);

        // Exit if invalid session count given
        if (maxClients < 1 || maxClients > MAX_WAIT_OBJECTS - 1) {
            Log::writeError("[IPC] Invalid number of sessions requested");
            this->error_ = true;
            return;
        }

        // Create server
        Handle serverHandle;
        this->serverName = smEncodeName(name.c_str());
        Result rc = smRegisterService(&serverHandle, this->serverName, false, maxClients);
        if (R_FAILED(rc)) {
            Log::writeError("[IPC] Couldn't create server: " + std::to_string(rc));
            return;
        }

        Log::writeSuccess("[IPC] Server started");
        this->handles.push_back(serverHandle);
    }

    void Server::parseRequest(Request & request) {
        // Fill request with default data
        uint8_t * base = static_cast<uint8_t *>(armGetTls());
        request.hipc = hipcParseRequest(base);
        request.in.cmdId = 0;
        request.in.arg = std::vector<uint8_t>();

        // Check request type and get cmd/args
        if (request.hipc.meta.type == CmifCommandType_Request) {
            Header * header = static_cast<Header *>(cmifGetAlignedDataStart(request.hipc.data.data_words, base));
            size_t dataSize = request.hipc.meta.num_data_words * 4;

            // Do nothing on an error
            if (!header || dataSize < sizeof(Header) || header->magic != CMIF_IN_HEADER_MAGIC) {
                Log::writeError("[IPC] Failed parsing request: bad header");
                return;
            }

            // Otherwise set request data
            request.in.cmdId = header->cmdId;
            if (dataSize > sizeof(Header)) {
                uint8_t * ptr = reinterpret_cast<uint8_t *>(header) + sizeof(Header);
                size_t size = dataSize - sizeof(Header);
                request.in.arg = std::vector<uint8_t>(ptr, ptr + size);
            }
        }

        // Create vector from send buffers
        if (request.hipc.meta.num_send_buffers > 0) {
            uint8_t * ptr = static_cast<uint8_t *>(hipcGetBufferAddress(request.hipc.data.send_buffers));
            size_t size = hipcGetBufferSize(request.hipc.data.send_buffers);
            request.in.data = std::vector<uint8_t>(ptr, ptr + size);
        }
        request.out.data = std::vector<uint8_t>();
        request.out.reply = std::vector<uint8_t>();
    }

    void Server::prepareResponse(uint32_t result, Request & request) {
        // Handle data buffer to reply with first
        if (!request.out.data.empty() && request.hipc.meta.num_recv_buffers > 0) {
            size_t size = hipcGetBufferSize(request.hipc.data.recv_buffers);
            if (request.out.data.size() < size) {
                size = request.out.data.size();
            }
            std::memcpy(hipcGetBufferAddress(request.hipc.data.recv_buffers), &request.out.data[0], size);
        }

        // Fill request
        uint8_t * base = static_cast<uint8_t *>(armGetTls());
        HipcRequest hipc = hipcMakeRequestInline(base,
            .type = CmifCommandType_Request,
            .num_data_words = static_cast<uint32_t>(sizeof(Header) + request.out.reply.size() + 0x10)/4,
        );

        Header * header = static_cast<Header *>(cmifGetAlignedDataStart(hipc.data_words, base));
        header->magic = CMIF_OUT_HEADER_MAGIC;
        header->result = result;

        // Finally append reply
        if (R_SUCCEEDED(result) && !request.out.reply.empty()) {
            std::memcpy(reinterpret_cast<uint8_t *>(header) + sizeof(Header), &request.out.reply[0], request.out.reply.size());
        }
    }

    bool Server::processSession(const int32_t index) {
        Request request;
        int tmp;

        // Get and parse request
        Result rc = svcReplyAndReceive(&tmp, &this->handles[index], 1, 0, UINT64_MAX);
        if (R_FAILED(rc)) {
            Log::writeError("[IPC] Couldn't receive request (closing handle): " + std::to_string(rc));
            svcCloseHandle(this->handles[index]);
            this->handles.erase(this->handles.begin() + index);
            return true;        // Return true as closing a session is valid behaviour
        }
        this->parseRequest(request);

        // Take action based on request type
        bool closeSession = false;
        switch (request.hipc.meta.type) {
            // Call handler to prepare response
            case CmifCommandType_Request: {
                uint32_t result = this->handler(request);
                this->prepareResponse(result, request);
                break;
            }

            // Prepare default response
            case CmifCommandType_Close:
                this->prepareResponse(0, request);
                closeSession = true;
                break;

            // Otherwise prepare error response
            default:
                Log::writeWarning("[IPC] Received unexpected CmifCommand");
                this->prepareResponse(MAKERESULT(11, 403), request);
                break;
        }

        // Send response
        rc = svcReplyAndReceive(&tmp, &this->handles[index], 0, this->handles[index], 0);
        if (rc == KERNELRESULT(TimedOut)) {
            rc = 0;
        }

        // Close session on error or close request
        if (R_FAILED(rc) || closeSession) {
            Log::writeInfo("Closing session " + std::to_string(index) + " due to error/request");
            svcCloseHandle(this->handles[index]);
            this->handles.erase(this->handles.begin() + index);
        }

        return (R_SUCCEEDED(rc));
    }

    bool Server::processNewSession() {
        Handle session;
        Result rc = svcAcceptSession(&session, this->handles[0]);
        if (R_SUCCEEDED(rc)) {
            // Check we have room
            if (this->handles.size() >= this->maxHandles) {
                Log::writeWarning("[IPC] Couldn't handle new session due to limit");
                svcCloseHandle(session);

            // Add session to vector
            } else {
                this->handles.push_back(session);
            }

            return true;
        }

        return false;
    }

    size_t Server::maxMessageSize() {
        return maxReplyBytes;
    }

    void Server::setRequestHandler(Handler f) {
        this->handler = f;
    }

    bool Server::process() {
        if (this->error_) {
            return false;
        }

        // Wait for a client to send a request/message
        int32_t handleIndex;
        Result rc = svcWaitSynchronization(&handleIndex, &this->handles[0], this->handles.size(), waitTimeout);
        if (R_VALUE(rc) == KERNELRESULT(TimedOut)) {
            return !this->error_;
        }

        if (R_SUCCEEDED(rc)) {
            // Check we're within range
            if (handleIndex < 0 || static_cast<uint32_t>(handleIndex) >= this->handles.size()) {
                Log::writeError("[IPC] svcWaitSynchronization returned out of range index: " + std::to_string(handleIndex));
                this->error_ = true;
                return false;
            }

            // If the index is not zero then we need to handle that client's request
            bool ok = true;
            if (handleIndex != 0) {
                ok = this->processSession(handleIndex);

            // Otherwise prepare for a new session
            } else {
                ok = this->processNewSession();
            }

            // Exit on an error
            if (!ok) {
                Log::writeInfo("[IPC] Failed to handle " + std::string(handleIndex == 0 ? "server" : "client " + std::to_string(handleIndex)) + " request");
                this->error_ = true;
            }
        }

        return !this->error_;
    }

    Server::~Server() {
        // Close all client handles
        for (size_t i = 1; i < this->handles.size(); i++) {
            svcCloseHandle(this->handles[i]);
        }

        // Finally close server handle
        if (!this->handles.empty()) {
            svcCloseHandle(this->handles[0]);
            Result rc = smUnregisterService(this->serverName);
            if (R_FAILED(rc)) {
                Log::writeError("[IPC] Couldn't unregister server: " + std::to_string(rc));
            }
        }
    }
}