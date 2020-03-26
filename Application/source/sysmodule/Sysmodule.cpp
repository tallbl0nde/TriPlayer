#include "Commands.h"
#include "Sysmodule.hpp"
#include "Utils.hpp"

#define PORT 3333
#define VOL_AMT 10

// Query/update variables this often
#define QUERY_MS 100

Sysmodule::Sysmodule() {
    // Get socket
    this->socket = -1;
    this->reconnect();
}

bool Sysmodule::isConnected() {
    return (version == SM_PROTOCOL_VERSION);
}

void Sysmodule::reconnect() {
    if (this->socket >= 0) {
        Utils::Socket::closeSocket(this->socket);
    }
    this->socket = Utils::Socket::createSocket(PORT);

    // Get protocol version
    Utils::Socket::writeToSocket(this->socket, std::to_string(VERSION));
    std::string str = Utils::Socket::readFromSocket(this->socket);
    if (str != "") {
        this->version = std::stoi(str);
    } else {
        this->version = -1;
    }
    if (this->version != SM_PROTOCOL_VERSION) {
        Utils::writeStdout("[SYSMODULE] [isReady()] Sysmodule version does not match!");
        this->version = -1;
    }
}

void Sysmodule::addToWQueue(std::string s) {
    queueMutex.lock();
    this->writeQueue.push(s);
    queueMutex.unlock();
}

void Sysmodule::updateState() {
    // Stop the loop if any of these fail (should only fail if socket/service is stopped/closed)
    bool loop = true;
    while (loop && this->version >= 0) {
        auto start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        // Write queued messages
        queueMutex.lock();
        while (writeQueue.size() > 0) {
            if (!Utils::Socket::writeToSocket(this->socket, writeQueue.front())) {
                loop = false;
            }
            writeQueue.pop();
            if (!loop) {
                break;
            }
        }
        queueMutex.unlock();

        // Update variables
        if (loop) {
            // loop = this->getSongID();
        }
        if (loop) {
            loop = this->getPosition_();
        }
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        Utils::writeStdout("Get pos took: " + std::to_string(time - start) + " milliseconds");
        if (loop) {
            loop = this->getStatus_();
        }
        time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        Utils::writeStdout("Get status took: " + std::to_string(time - start) + " milliseconds");

        // Determine how long to sleep (in ms)
        auto end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        Utils::writeStdout("Sysmodule loop took: " + std::to_string(end - start) + " milliseconds");
        int sleep = QUERY_MS - (end - start);
        if (sleep > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }
}

bool Sysmodule::getSongID() {
    // Request song ID
    if (!Utils::Socket::writeToSocket(this->socket, std::to_string(GETSONG))) {
        return false;
    }

    // Read ID
    std::string str = Utils::Socket::readFromSocket(this->socket);
    if (str == "") {
        return false;
    }

    this->currentID = std::stoi(str);
    return true;
}

bool Sysmodule::getStatus_() {
    // Request status
    if (!Utils::Socket::writeToSocket(this->socket, std::to_string(GETSTATUS))) {
        this->status_ = Error;
        return false;
    }
    
    // Read status
    std::string str = Utils::Socket::readFromSocket(this->socket);
    if (str == "") {
        this->status_ = Error;
        return false;
    }

    SM_Status st = (SM_Status)std::stoi(str);
    switch (st) {
        case ERROR:
            this->status_ = Error;
            break;

        case PLAYING:
            this->status_ = Playing;
            break;

        case PAUSED:
            this->status_ = Paused;
            break;

        case STOPPED:
            this->status_ = Stopped;
            break;
    }
    return true;
}

bool Sysmodule::getPosition_() {
    // Request position
    if (!Utils::Socket::writeToSocket(this->socket, std::to_string(GETPOSITION))) {
        return false;
    }

    // Read position
    std::string str = Utils::Socket::readFromSocket(this->socket);
    if (str == "") {
        return false;
    }

    this->position_ = std::stod(str);
    return true;
}

void Sysmodule::resumePlayback() {
    this->addToWQueue(std::to_string(RESUME));
}

void Sysmodule::pausePlayback() {
    this->addToWQueue(std::to_string(PAUSE));
}

void Sysmodule::previousSong() {
    this->addToWQueue(std::to_string(PREVIOUS));
}

void Sysmodule::nextSong() {
    this->addToWQueue(std::to_string(NEXT));
}

void Sysmodule::setVolume(int v) {
    this->addToWQueue(std::to_string(SETVOLUME) + std::string(1, SM_DELIMITER) + std::to_string(v));
}

void Sysmodule::playSong(SongID i) {
    this->addToWQueue(std::to_string(PLAY) + std::string(1, SM_DELIMITER) + std::to_string(i));
}

void Sysmodule::addToQueue(SongID i) {
    this->addToWQueue(std::to_string(ADDTOQUEUE) + std::string(1, SM_DELIMITER) + std::to_string(i));
}

void Sysmodule::removeFromQueue(SongID i) {
    this->addToWQueue(std::to_string(REMOVEFROMQUEUE) + std::string(1, SM_DELIMITER) + std::to_string(i));
}

// bool Sysmodule::getQueue(std::vector<SongID> & q) {
//     // Request queue
//     if (!Utils::Socket::writeToSocket(this->socket, std::to_string(GETQUEUE))) {
//         return false;
//     }

//     // Read queue
//     std::string str = Utils::Socket::readFromSocket(this->socket);
//     if (str == "") {
//         return false;
//     }

//     // Split received string and populate vector
//     q.clear();
//     unsigned int lpos = 0;
//     unsigned int pos;
//     while ((pos = str.find(SM_DELIMITER, lpos)) != std::string::npos) {
//         q.push_back(std::stoi(str.substr(lpos, pos)));
//         lpos = pos + 1;
//     }

//     return true;
// }

// bool Sysmodule::setQueue(std::vector<SongID> q) {
//     // Create string from vector
//     std::string str = std::to_string(SETQUEUE);
//     for (size_t i = 0; i < q.size(); i++) {
//         str += std::string(1, SM_DELIMITER);
//         str += q[i];
//     }

//     return Utils::Socket::writeToSocket(this->socket, str);
// }

void Sysmodule::shuffleQueue() {
    this->addToWQueue(std::to_string(SHUFFLE));
}

void Sysmodule::repeatOn() {
    this->addToWQueue(std::to_string(SETREPEAT));
}

void Sysmodule::repeatOff() {
    this->addToWQueue(std::to_string(SETREPEAT));
}

SongID Sysmodule::playingID() {
    return this->currentID;
}

double Sysmodule::position() {
    return this->position_;
}

PlaybackStatus Sysmodule::status() {
    return this->status_;
}

void Sysmodule::finish() {
    this->version = -1;
}

bool Sysmodule::reset() {
    return Utils::Socket::writeToSocket(this->socket, std::to_string(RESET));
}

Sysmodule::~Sysmodule() {
    Utils::Socket::closeSocket(this->socket);
}