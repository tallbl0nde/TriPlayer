#include "Commands.hpp"
#include "Sysmodule.hpp"
#include "Utils.hpp"

#define PORT 3333
#define VOL_AMT 10

Sysmodule::Sysmodule() {
    // Get socket
    this->socket = -1;
    this->reconnect();
}

bool Sysmodule::isReady() {
    // Check versions match
    if (!Utils::Socket::writeToSocket(this->socket, std::to_string(VERSION))) {
        // Error writing
        return false;
    }

    std::string str = Utils::Socket::readFromSocket(this->socket);
    if (str == "") {
        // Error reading
        return false;
    }

    if (std::stoi(str) != SM_PROTOCOL_VERSION) {
        Utils::writeStdout("[SYSMODULE] [isReady()] Sysmodule version does not match!");
        return false;
    }

    return true;
}

void Sysmodule::reconnect() {
    if (this->socket >= 0) {
        Utils::Socket::closeSocket(this->socket);
    }
    this->socket = Utils::Socket::createSocket(PORT);
}

bool Sysmodule::resumePlayback() {
    return Utils::Socket::writeToSocket(this->socket, std::to_string(RESUME));
}

bool Sysmodule::pausePlayback() {
    return Utils::Socket::writeToSocket(this->socket, std::to_string(PAUSE));
}

bool Sysmodule::previousSong() {
    return Utils::Socket::writeToSocket(this->socket, std::to_string(PREVIOUS));
}

bool Sysmodule::nextSong() {
    return Utils::Socket::writeToSocket(this->socket, std::to_string(NEXT));
}

bool Sysmodule::decreaseVolume() {
    // Decrease volume and set absolute value
    if (!Utils::Socket::writeToSocket(this->socket, std::to_string(GETVOLUME))) {
        return false;
    }
    std::string str = Utils::Socket::readFromSocket(this->socket);
    if (str == "") {
        return false;
    }
    int vol = std::stoi(str);
    vol -= VOL_AMT;
    if (vol < 0) {
        vol = 0;
    }

    return this->setVolume(vol);
}

bool Sysmodule::increaseVolume() {
    // Increase volume and set absolute value
    if (!Utils::Socket::writeToSocket(this->socket, std::to_string(GETVOLUME))) {
        return false;
    }
    std::string str = Utils::Socket::readFromSocket(this->socket);
    if (str == "") {
        return false;
    }
    int vol = std::stod(str);
    vol -= VOL_AMT;
    if (vol > 100) {
        vol = 100;
    }

    return this->setVolume(vol);
}

bool Sysmodule::setVolume(int v) {
    return Utils::Socket::writeToSocket(this->socket, std::to_string(SETVOLUME) + std::string(1, SM_DELIMITER) + std::to_string(v));
}

bool Sysmodule::playSong(SongID i) {
    return Utils::Socket::writeToSocket(this->socket, std::to_string(PLAY) + std::string(1, SM_DELIMITER) + std::to_string(i));
}

bool Sysmodule::addToQueue(SongID i) {
    return Utils::Socket::writeToSocket(this->socket, std::to_string(ADDTOQUEUE) + std::string(1, SM_DELIMITER) + std::to_string(i));
}

bool Sysmodule::removeFromQueue(SongID i) {
    return Utils::Socket::writeToSocket(this->socket, std::to_string(REMOVEFROMQUEUE) + std::string(1, SM_DELIMITER) + std::to_string(i));
}

bool Sysmodule::getQueue(std::vector<SongID> & q) {
    // Request queue
    if (!Utils::Socket::writeToSocket(this->socket, std::to_string(GETQUEUE))) {
        return false;
    }

    // Read queue
    std::string str = Utils::Socket::readFromSocket(this->socket);
    if (str == "") {
        return false;
    }

    // Split received string and populate vector
    q.clear();
    unsigned int lpos = 0;
    unsigned int pos;
    while ((pos = str.find(SM_DELIMITER, lpos)) != std::string::npos) {
        q.push_back(std::stoi(str.substr(lpos, pos)));
        lpos = pos + 1;
    }

    return true;
}

bool Sysmodule::setQueue(std::vector<SongID> q) {
    // Create string from vector
    std::string str = std::to_string(SETQUEUE);
    for (size_t i = 0; i < q.size(); i++) {
        str += std::string(1, SM_DELIMITER);
        str += q[i];
    }

    return Utils::Socket::writeToSocket(this->socket, str);
}

bool Sysmodule::shuffleQueue() {
    return Utils::Socket::writeToSocket(this->socket, std::to_string(SHUFFLE));
}

bool Sysmodule::repeatOn() {
    return Utils::Socket::writeToSocket(this->socket, std::to_string(REPEATON));
}

bool Sysmodule::repeatOff() {
    return Utils::Socket::writeToSocket(this->socket, std::to_string(REPEATOFF));
}

bool Sysmodule::getCurrentSong(SongID * i) {
    // Request song ID
    if (!Utils::Socket::writeToSocket(this->socket, std::to_string(GETSONG))) {
        *i = -1;
        return false;
    }

    // Read ID
    std::string str = Utils::Socket::readFromSocket(this->socket);
    if (str == "") {
        *i = -2;
        return false;
    }

    *i = std::stoi(str);
    return true;
}

bool Sysmodule::getStatus(SysmoduleStatus * s) {
    // Request status
    if (!Utils::Socket::writeToSocket(this->socket, std::to_string(GETSTATUS))) {
        *s = ERROR;
        return false;
    }

    // Read status
    std::string str = Utils::Socket::readFromSocket(this->socket);
    if (str == "") {
        *s = ERROR;
        return false;
    }

    *s = (SysmoduleStatus)std::stoi(str);
    return true;
}

bool Sysmodule::reset() {
    return Utils::Socket::writeToSocket(this->socket, std::to_string(RESET));
}

Sysmodule::~Sysmodule() {
    Utils::Socket::closeSocket(this->socket);
}