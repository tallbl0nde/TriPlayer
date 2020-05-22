#include <cstring>
#include <limits>
#include "Log.hpp"
#include <mutex>
#include "Protocol.hpp"
#include "Socket.hpp"
#include "Sysmodule.hpp"

// Macro for adding delimiter
#define DELIM std::string(1, Protocol::Delimiter)
// Number of seconds between updating state (automatically)
#define UPDATE_DELAY 0.1

void Sysmodule::addToWriteQueue(std::string s, std::function<void(std::string)> f) {
    if (this->error_) {
        return;
    }

    std::scoped_lock<std::shared_mutex> mtx(this->writeMutex);
    this->writeQueue.push(std::pair< std::string, std::function<void(std::string)> >(s, f));
}

Sysmodule::Sysmodule() {
    this->currentSong_ = -1;
    this->error_ = true;
    this->exit_ = false;
    this->lastUpdateTime = std::chrono::steady_clock::now();
    this->position_ = 0.0;
    this->queueChanged_ = false;
    this->queueSize_ = 0;
    this->repeatMode_ = RepeatMode::Off;
    this->shuffleMode_ = ShuffleMode::Off;
    this->songIdx_ = 0;
    this->status_ = PlaybackStatus::Stopped;
    this->volume_ = 100.0;

    // Get socket
    this->socket = -1;
    this->reconnect();

    // Fetch queue at launch
    this->sendGetQueue(0, 65535);
}

bool Sysmodule::error() {
    return this->error_;
}

void Sysmodule::reconnect() {
    std::scoped_lock<std::shared_mutex> mtx(this->writeMutex);

    // Create and setup socket
    if (this->socket >= 0) {
        Utils::Socket::closeSocket(this->socket);
    }
    this->socket = Utils::Socket::createSocket(Protocol::Port);
    Utils::Socket::setTimeout(this->socket, Protocol::Timeout);

    // Get protocol version
    Utils::Socket::writeToSocket(this->socket, std::to_string((int)Protocol::Command::Version));
    std::string str = Utils::Socket::readFromSocket(this->socket);
    if (str.length() > 0) {
        int ver = std::stoi(str);
        if (ver != Protocol::Version) {
            Log::writeError("[SYSMODULE] Versions do not match!");
            this->error_ = true;
            return;
        }
    } else {
        Log::writeError("[SYSMODULE] An error occurred getting version!");
        this->error_ = true;
        return;
    }

    Log::writeSuccess("[SYSMODULE] Socket (re)connected successfully!");
    this->error_ = false;
}

void Sysmodule::process() {
    // Loop until we want to exit
    while (!this->exit_) {
        // Sleep if an error occurred
        if (this->error_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        // First process anything on the queue
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        std::unique_lock<std::shared_mutex> mtx(this->writeMutex);
        while (!this->writeQueue.empty()) {
            if (!Utils::Socket::writeToSocket(this->socket, this->writeQueue.front().first)) {
                this->error_ = true;
            } else {
                std::string str = Utils::Socket::readFromSocket(this->socket);
                if (str.length() == 0) {
                    this->error_ = true;
                } else {
                    std::function<void(std::string)> queuedFunc = this->writeQueue.front().second;
                    mtx.unlock();
                    queuedFunc(str);
                    mtx.lock();
                    this->writeQueue.pop();
                }
            }

            // Clear queue if an error occurred
            if (this->error_) {
                Log::writeError("[SYSMODULE] Error occurred while processing queue - cleared queue");
                this->writeQueue = std::queue< std::pair<std::string, std::function<void(std::string)> > >();
                continue;
            }
        }
        mtx.unlock();

        // Check if we need to log to save cycles
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("Sysmodule update took: " + std::to_string(std::chrono::duration_cast< std::chrono::duration<double> >(std::chrono::steady_clock::now() - now).count()) + " seconds");
        }

        // Check if variables need to be updated
        now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast< std::chrono::duration<double> >(now - this->lastUpdateTime).count() > UPDATE_DELAY) {
            this->sendGetPosition();
            this->sendGetQueueSize();
            this->sendGetRepeat();
            this->sendGetShuffle();
            this->sendGetSong();
            this->sendGetSongIdx();
            this->sendGetStatus();
            this->sendGetVolume();
            this->lastUpdateTime = now;

        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

SongID Sysmodule::currentSong() {
    return this->currentSong_;
}

double Sysmodule::position() {
    return this->position_;
}

bool Sysmodule::queueChanged() {
    if (this->queueChanged_) {
        this->queueChanged_ = false;
        return true;
    }

    return false;
}

std::vector<SongID> Sysmodule::queue() {
    std::shared_lock<std::shared_mutex> mtx(this->queueMutex);

    // Return a copy of the vector (this is likely slow - there must be a better way?)
    return this->queue_;
}

size_t Sysmodule::queueSize() {
    return this->queueSize_;
}

RepeatMode Sysmodule::repeatMode() {
    return this->repeatMode_;
}

ShuffleMode Sysmodule::shuffleMode() {
    return this->shuffleMode_;
}

size_t Sysmodule::songIdx() {
    return this->songIdx_;
}

PlaybackStatus Sysmodule::status() {
    return this->status_;
}

double Sysmodule::volume() {
    return this->volume_;
}

size_t Sysmodule::waitSongIdx() {
    std::atomic<bool> done = false;

    // Query song idx
    this->addToWriteQueue(std::to_string((int)Protocol::Command::QueueIdx), [this, &done](std::string s) {
        this->songIdx_ = std::stoi(s);
        done = true;
    });

    // Block until done
    while (!done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if (this->error_) {
            return std::numeric_limits<size_t>::max();
            break;
        }
    }

    return this->songIdx_;
}

void Sysmodule::sendResume() {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::Resume), [this](std::string s) {
        this->currentSong_ = std::stoi(s);
    });
}

void Sysmodule::sendPause() {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::Pause), [this](std::string s) {
        this->currentSong_ = std::stoi(s);
    });
}

void Sysmodule::sendPrevious() {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::Previous), [this](std::string s) {
        this->currentSong_ = std::stoi(s);
    });
}

void Sysmodule::sendNext() {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::Next), [this](std::string s) {
        this->currentSong_ = std::stoi(s);
    });
}

void Sysmodule::sendGetVolume() {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::GetVolume), [this](std::string s) {
        this->volume_ = std::stod(s);
    });
}

void Sysmodule::sendSetVolume(const double v) {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::SetVolume) + DELIM + std::to_string(v), [this](std::string s) {
        this->volume_ = std::stod(s);
    });
}

void Sysmodule::sendSetSongIdx(const size_t id) {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::SetQueueIdx) + DELIM + std::to_string(id), [this](std::string s) {
        this->currentSong_ = std::stoi(s);
    });
}

void Sysmodule::sendGetSongIdx() {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::QueueIdx), [this](std::string s) {
        this->songIdx_ = std::stoi(s);
    });
}

void Sysmodule::sendGetQueueSize() {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::QueueSize), [this](std::string s) {
        this->queueSize_ = std::stoi(s);
    });
}

void Sysmodule::sendAddToQueue(const SongID id) {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::AddToQueue) + DELIM + std::to_string(id), [this, id](std::string s) {
        if (std::stoi(s) != id) {
            // On an error get the whole queue again
            this->sendGetQueue(0, 65535);

        } else {
            // On success append to local queue
            std::scoped_lock<std::shared_mutex> mtx(this->queueMutex);
            this->queue_.push_back(id);
        }
    });
}

void Sysmodule::sendRemoveFromQueue(const size_t pos) {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::RemoveFromQueue) + DELIM + std::to_string(pos), [this, pos](std::string s) {
        if (std::stoul(s) != pos) {
            // On an error get the whole queue again
            this->sendGetQueue(0, 65535);

        } else {
            // On success remove from local queue
            std::scoped_lock<std::shared_mutex> mtx(this->queueMutex);
            this->queue_.erase(this->queue_.begin() + pos);
        }
    });
}

void Sysmodule::sendGetQueue(const size_t s, const size_t e) {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::GetQueue) + DELIM + std::to_string(s) + DELIM + std::to_string(e), [this](std::string s) {
        // Add each token in string
        std::scoped_lock<std::shared_mutex> mtx(this->queueMutex);
        this->queue_.clear();
        char * str = strdup(s.c_str());
        char * tok = strtok(str, &Protocol::Delimiter);
        while (tok != nullptr) {
            this->queue_.push_back(strtol(tok, nullptr, 10));
            tok = strtok(nullptr, &Protocol::Delimiter);
        }
        free(str);
        this->queueChanged_ = true;
    });
}

void Sysmodule::sendSetQueue(const std::vector<SongID> & q) {
    // Construct string first
    std::string seq;
    size_t size = q.size();
    for (size_t i = 0; i < size; i++) {
        seq.push_back(Protocol::Delimiter);
        seq += std::to_string(q[i]);
    }

    // Edit local queue before sending to sysmodule
    this->queue_ = q;
    this->queueChanged_ = true;

    this->addToWriteQueue(std::to_string((int)Protocol::Command::SetQueue) + seq, [this, size](std::string s) {
        if (std::stoul(s) != size) {
            // On an error get the whole queue again
            this->sendGetQueue(0, 65535);
        }
    });
}

void Sysmodule::sendGetRepeat() {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::GetRepeat), [this](std::string s) {
        RepeatMode rm = RepeatMode::Off;
        switch ((Protocol::Repeat)std::stoi(s)) {
            case Protocol::Repeat::Off:
                rm = RepeatMode::Off;
                break;

            case Protocol::Repeat::One:
                rm = RepeatMode::One;
                break;

            case Protocol::Repeat::All:
                rm = RepeatMode::All;
                break;
        }
        this->repeatMode_ = rm;
    });
}

void Sysmodule::sendSetRepeat(const RepeatMode m) {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::SetRepeat) + DELIM + std::to_string((int)m), [this, m](std::string s) {
        RepeatMode rm;
        switch ((Protocol::Repeat)std::stoi(s)) {
            case Protocol::Repeat::Off:
                rm = RepeatMode::Off;
                break;

            case Protocol::Repeat::One:
                rm = RepeatMode::One;
                break;

            case Protocol::Repeat::All:
                rm = RepeatMode::All;
                break;
        }

        if (rm != m) {
            // Handle error here

        } else {
            this->repeatMode_ = rm;
        }
    });
}

void Sysmodule::sendGetShuffle() {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::GetShuffle), [this](std::string s) {
        this->shuffleMode_ = ((Protocol::Shuffle)std::stoi(s) == Protocol::Shuffle::Off ? ShuffleMode::Off : ShuffleMode::On);
    });
}

void Sysmodule::sendSetShuffle(const ShuffleMode m) {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::SetShuffle) + DELIM + std::to_string((int)m), [this, m](std::string s) {
        ShuffleMode sm = ((Protocol::Shuffle)std::stoi(s) == Protocol::Shuffle::Off ? ShuffleMode::Off : ShuffleMode::On);
        if (sm != m) {
            // Handle error here

        } else {
            this->sendGetQueue(0, 65535);
            this->shuffleMode_ = sm;
        }
    });
}

void Sysmodule::sendGetSong() {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::GetSong), [this](std::string s) {
        this->currentSong_ = std::stoi(s);
    });
}

void Sysmodule::sendGetStatus() {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::GetStatus), [this](std::string s) {
        PlaybackStatus ps = PlaybackStatus::Error;
        switch ((Protocol::Status)std::stoi(s)) {
            case Protocol::Status::Error:
                ps = PlaybackStatus::Error;
                break;

            case Protocol::Status::Playing:
                ps = PlaybackStatus::Playing;
                break;

            case Protocol::Status::Paused:
                ps = PlaybackStatus::Paused;
                break;

            case Protocol::Status::Stopped:
                ps = PlaybackStatus::Stopped;
                break;
        }
        this->status_ = ps;
    });
}

void Sysmodule::sendGetPosition() {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::GetPosition), [this](std::string s) {
        this->position_ = std::stod(s);
    });
}

void Sysmodule::sendSetPosition(double pos) {
    this->position_ = pos;
    this->addToWriteQueue(std::to_string((int)Protocol::Command::SetPosition) + DELIM + std::to_string(pos), [this](std::string s) {
        this->position_ = std::stod(s);
    });
}

void Sysmodule::sendReset() {
    this->addToWriteQueue(std::to_string((int)Protocol::Command::Reset), [this](std::string s) {

    });
}

void Sysmodule::exit() {
    this->exit_ = true;
}

Sysmodule::~Sysmodule() {
    if (this->socket >= 0) {
        Utils::Socket::closeSocket(this->socket);
    }
}