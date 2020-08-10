#include <cstring>
#include <errno.h>
#include "Log.hpp"
#include "socket/Transfer.hpp"
#include <sys/socket.h>

// Number of failures before considering disconnected
#define MAX_FAILURES 5
// Number of bytes to read in one go
#define READ_BYTES 1000

namespace Socket {
    Transfer::Transfer(int fd) {
        this->connected = true;
        this->failures = 0;
        this->transferSocket = fd;
    }

    std::string Transfer::getMessageFromBuffer() {
        // Pop string
        std::string str = this->msgBuffer.front();
        this->msgBuffer.pop();

        // Log if necessary
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("[SOCKET] Read data '" + str + "'");
        }

        return str;
    }

    bool Transfer::isConnected() {
        return this->connected;
    }

    std::string Transfer::readMessage() {
        // Simply return buffered message if there is one
        if (!this->msgBuffer.empty()) {
            return this->getMessageFromBuffer();
        }

        // Read until entire message found
        std::vector<char> buffer;
        do {
            // Read into temporary buffer
            char buf[READ_BYTES];
            int rd = read(this->transferSocket, buf, READ_BYTES);

            // Copy into other buffer if read was successful
            if (rd > 0) {
                this->failures = 0;
                for (int i = 0; i < rd; i++) {
                    buffer.push_back(buf[i]);
                }

            // Increment failure counter if it failed
            } else if (rd < 0) {
                Log::writeError("[SOCKET] Error occurred reading from socket: " + std::to_string(errno));
                buffer.clear();
                failures++;
                if (failures >= MAX_FAILURES) {
                    this->connected = false;
                    return "";
                }

            // If we know we lost connection simply indicate loss of connection
            } else {
                Log::writeInfo("[SOCKET] Lost connection while waiting to read");
                buffer.clear();
                this->connected = false;
                return "";
            }
        } while (buffer.size() == 0 || buffer[buffer.size() - 1] != '\0');

        // Break up if multiple messages were received
        size_t s = 0;
        for (size_t c = 0; c < buffer.size(); c++) {
            // Once the end is found add to buffer
            if (buffer[c] == '\0') {
                std::string str(buffer.begin() + s, buffer.begin() + c);
                str.shrink_to_fit();
                this->msgBuffer.push(str);
                s = c + 1;
            }
        }

        // Return a message in the cache (there will be one at this point!)
        std::string str;
        if (!this->msgBuffer.empty()) {
            str = this->getMessageFromBuffer();
        } else {
            str = "";
        }
        return str;
    }

    bool Transfer::writeMessage(const std::string & str) {
        // Get C-String
        const char * tmp = str.c_str();
        int len = strlen(tmp) + 1;

        // Actually write the data
        if (write(this->transferSocket, tmp, len) != len) {
            Log::writeError("[SOCKET] Failed to write data: " + std::to_string(errno));
            return false;
        }

        // Log data if enabled
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("[SOCKET] Wrote data: '" + str + "'");
        }
        return true;
    }

    Transfer::~Transfer() {
        close(this->transferSocket);
    }
};