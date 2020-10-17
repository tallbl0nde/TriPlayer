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
        ::Result rc = smRegisterService(&serverHandle, this->serverName, false, maxClients);
        if (R_FAILED(rc)) {
            Log::writeError("[IPC] Couldn't create server: " + std::to_string(rc));
            return;
        }

        Log::writeSuccess("[IPC] Server started");
        this->handles.push_back(serverHandle);
    }

    bool Server::processSession(const int32_t index) {
        int tmp;

        // Wait for request
        ::Result rc = svcReplyAndReceive(&tmp, &this->handles[index], 1, 0, UINT64_MAX);
        if (R_FAILED(rc)) {
            Log::writeError("[IPC] Couldn't receive request (closing handle): " + std::to_string(rc));
            svcCloseHandle(this->handles[index]);
            this->handles.erase(this->handles.begin() + index);
            return true;        // Return true as closing a session is valid behaviour
        }

        // Create object from received data
        Request * request = Request::fromTLS();
        if (!request) {
            Log::writeError("[IPC] An error occurred creating the request object (most likely bad header magic)");
            return false;
        }

        // Take action based on request type
        bool closeSession = false;
        switch (request->type()) {
            // Call handler to prepare response
            case Request::Type::Request: {
                uint32_t result = this->handler(request);
                request->setResult(result);
                request->toResponseTLS();
                break;
            }

            // Prepare default response
            case Request::Type::Close:
                request->setResult(0);
                request->toResponseTLS();
                closeSession = true;
                break;

            // Otherwise prepare error response
            default:
                Log::writeInfo("[IPC] Received unexpected CmifCommand");
                request->setResult(MAKERESULT(11, 403));
                request->toResponseTLS();
                break;
        }

        // Send response and delete object
        rc = svcReplyAndReceive(&tmp, &this->handles[index], 0, this->handles[index], 0);
        if (rc == KERNELRESULT(TimedOut)) {
            rc = 0;
        }
        delete request;

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
        ::Result rc = svcAcceptSession(&session, this->handles[0]);
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

    void Server::setRequestHandler(Handler f) {
        this->handler = f;
    }

    bool Server::process() {
        if (this->error_) {
            return false;
        }

        // Wait for a client to send a request/message
        int32_t handleIndex;
        ::Result rc = svcWaitSynchronization(&handleIndex, &this->handles[0], this->handles.size(), waitTimeout);
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
            ::Result rc = smUnregisterService(this->serverName);
            if (R_FAILED(rc)) {
                Log::writeError("[IPC] Couldn't unregister server: " + std::to_string(rc));
            }
        }
    }
}