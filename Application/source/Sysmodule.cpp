#include "ipc/TriPlayer.hpp"
#include <limits>
#include "Log.hpp"
#include "Sysmodule.hpp"
#include "utils/NX.hpp"

// Program ID of sysmodule
#define PROGRAM_ID 0x4200000000000FFF

// Number of seconds between updating state (automatically)
#define UPDATE_DELAY 0.1

bool Sysmodule::addToIpcQueue(std::function<bool()> f) {
    if (this->error_ != Error::None) {
        return false;
    }

    std::scoped_lock<std::mutex> mtx(this->ipcMutex);
    this->ipcQueue.push(f);
    return true;
}

Sysmodule::Sysmodule() {
    // Attempt connection on creation
    this->connected_ = false;
    this->error_ = Error::Unknown;
    this->limit_ = -1;
    this->reconnect();

    // Initialize all variables
    this->currentSong_ = -1;
    this->exit_ = false;
    this->keepPosition = false;
    this->keepVolume = false;
    this->lastUpdateTime = std::chrono::steady_clock::now();
    this->playingFrom_ = "";
    this->position_ = 0.0;
    this->queueChanged_ = false;
    this->queueSize_ = 0;
    this->repeatMode_ = RepeatMode::Off;
    this->shuffleMode_ = ShuffleMode::Off;
    this->songIdx_ = 0;
    this->subQueueChanged_ = false;
    this->status_ = PlaybackStatus::Stopped;
    this->volume_ = 100.0;

    // Fetch queue at launch
    this->sendGetQueue();
    this->sendGetSubQueue();
}

Sysmodule::Error Sysmodule::error() {
    return this->error_;
}

bool Sysmodule::launch() {
    return Utils::NX::launchProgram(PROGRAM_ID);
}

void Sysmodule::reconnect() {
    std::scoped_lock<std::mutex> mtx(this->ipcMutex);

    // Clean up IPC if connected
    if (this->connected_) {
        Log::writeInfo("[SYSMODULE] Closing connection");
        TriPlayer::exit();
    }

    // Reinitialize
    this->connected_ = TriPlayer::initialize();
    if (!this->connected_) {
        this->error_ = Error::NotConnected;
        Log::writeError("[SYSMODULE] Couldn't connect to sysmodule");
        return;
    }

    // Check versions match
    std::string sysVer = "";
    if (!TriPlayer::getVersion(sysVer)) {
        this->error_ = Error::Unknown;
        Log::writeError("[SYSMODULE] Failed to get sysmodule version");
        return;
    }
    if (sysVer != std::string(VER_STRING)) {
        this->error_ = Error::DifferentVersion;
        Log::writeError("[SYSMODULE] Version mismatch! App: " + std::string(VER_STRING) + ", Sys: " + sysVer);
        return;
    }

    // If we reach here we're connected successfully!
    Log::writeSuccess("[SYSMODULE] Connection established!");
    this->error_ = Error::None;
}

bool Sysmodule::terminate() {
    // Lock IPC mutex so we can clear the write queue and add stop signal
    std::unique_lock<std::mutex> mtx(this->ipcMutex);
    this->ipcQueue = std::queue< std::function<bool()> >();
    this->error_ = Error::None;

    // Send the stop signal, and if successful clear the queue and set error
    // so no more commands are attempted to be sent
    bool done = false;
    bool ok = false;
    this->ipcQueue.push([this, &done, &ok]() -> bool {
        ok = TriPlayer::stopSysmodule();
        if (ok) {
            this->error_ = Error::NotConnected;
            std::scoped_lock<std::mutex> mtx(this->ipcMutex);
            this->ipcQueue = std::queue< std::function<bool()> >();
        }
        done = true;
        return ok;
    });

    // Now wait for function to run and return result!
    mtx.unlock();
    while (!done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if (this->error_ != Error::None) {
            break;
        }
    }

    // Wait for sysmodule to stop
    while (Utils::NX::runningProgram(PROGRAM_ID)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return ok;
}

void Sysmodule::setQueueLimit(int limit) {
    this->limit_ = limit;
}

void Sysmodule::process() {
    // Loop until we want to exit
    while (!this->exit_) {
        // Sleep if an error occurred (this way if the app asks it to reconnect it will continue communicating)
        if (this->error_ != Error::None) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        // First process commands on the write queue
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        std::unique_lock<std::mutex> mtx(this->ipcMutex);
        while (!this->ipcQueue.empty()) {
            // Get the first command on the queue
            std::function<bool()> func = this->ipcQueue.front();
            this->ipcQueue.pop();

            // Execute it and handle errors
            mtx.unlock();
            bool ok = func();
            mtx.lock();

            if (!ok) {
                this->error_ = Error::Unknown;
                Log::writeError("[SYSMODULE] Error occurred while processing queue");
                this->ipcQueue = std::queue< std::function<bool()> >();
            }
        }
        mtx.unlock();

        // Check if variables need to be updated
        now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast< std::chrono::duration<double> >(now - this->lastUpdateTime).count() > UPDATE_DELAY) {
            this->sendGetPlayingFrom();
            this->sendGetPosition();
            this->sendGetQueueSize();
            this->sendGetRepeat();
            this->sendGetShuffle();
            this->sendGetSong();
            this->sendGetSongIdx();
            this->sendGetSubQueueSize();
            this->sendGetStatus();
            this->sendGetVolume();
            this->lastUpdateTime = now;

        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

SongID Sysmodule::currentSong() {
    return this->currentSong_;
}

std::string Sysmodule::playingFrom() {
    std::scoped_lock<std::mutex> mtx(this->playingFromMutex);
    // If the string contains a space it's "empty"
    return (this->playingFrom_ == " " ? "" : this->playingFrom_);
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
    std::scoped_lock<std::mutex> mtx(this->queueMutex);
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

bool Sysmodule::subQueueChanged() {
    if (this->subQueueChanged_) {
        this->subQueueChanged_ = false;
        return true;
    }

    return false;
}

std::vector<SongID> Sysmodule::subQueue() {
    std::scoped_lock<std::mutex> mtx(this->subQueueMutex);
    return this->subQueue_;
}

size_t Sysmodule::subQueueSize() {
    return this->subQueueSize_;
}

PlaybackStatus Sysmodule::status() {
    return this->status_;
}

double Sysmodule::volume() {
    return this->volume_;
}

bool Sysmodule::waitRequestDBLock() {
    std::atomic<bool> done = false;

    this->addToIpcQueue([&done]() -> bool {
        bool b = TriPlayer::requestDatabaseLock();
        done = true;
        return b;
    });

    // Block until done
    while (!done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if (this->error_ != Error::None) {
            return false;
        }
    }
    return true;
}

bool Sysmodule::waitReset() {
    std::atomic<bool> done = false;

    this->addToIpcQueue([&done]() -> bool {
        bool b = TriPlayer::reset();
        done = true;
        return b;
    });

    // Block until done
    while (!done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if (this->error_ != Error::None) {
            return false;
        }
    }

    return true;
}

size_t Sysmodule::waitSongIdx() {
    std::atomic<bool> done = false;

    // Query song idx
    this->addToIpcQueue([this, &done]() -> bool {
        size_t idx;
        bool b = TriPlayer::getQueueIdx(idx);
        this->songIdx_ = idx;
        done = true;
        return b;
    });

    // Block until done
    while (!done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if (this->error_ != Error::None) {
            return std::numeric_limits<size_t>::max();
            break;
        }
    }

    return this->songIdx_;
}

void Sysmodule::sendResume() {
    this->addToIpcQueue([]() -> bool {
        return TriPlayer::resume();
    });
}

void Sysmodule::sendPause() {
    this->addToIpcQueue([]() -> bool {
        return TriPlayer::pause();
    });
}

void Sysmodule::sendPrevious() {
    this->addToIpcQueue([]() -> bool {
        return TriPlayer::previous();
    });
}

void Sysmodule::sendNext() {
    this->addToIpcQueue([]() -> bool {
        return TriPlayer::next();
    });
}

void Sysmodule::sendGetVolume() {
    this->addToIpcQueue([this]() -> bool {
        double vol;
        bool b = TriPlayer::getVolume(vol);
        if (b && !this->keepVolume) {
            this->volume_ = vol;
        }
        return b;
    });
}

void Sysmodule::sendSetVolume(const double v) {
    this->keepVolume = true;
    this->volume_ = v;
    this->addToIpcQueue([this, v]() -> bool {
        bool b = TriPlayer::setVolume(v);
        this->keepVolume = false;
        return b;
    });
}

void Sysmodule::sendMute() {
    this->addToIpcQueue([]() -> bool {
        return TriPlayer::mute();
    });
}

void Sysmodule::sendUnmute() {
    this->addToIpcQueue([this]() -> bool {
        double vol = 0;
        bool b = TriPlayer::unmute(vol);
        if (b) {
            this->volume_ = vol;
        }
        return b;
    });
}

void Sysmodule::sendGetSubQueue() {
    this->addToIpcQueue([this]() -> bool {
        std::vector<SongID> ids;
        bool b = TriPlayer::getSubQueue(ids);
        if (b) {
            std::scoped_lock<std::mutex> mtx(this->subQueueMutex);
            this->subQueue_ = ids;
            this->subQueueChanged_ = true;
        }
        return b;
    });
}

void Sysmodule::sendGetSubQueueSize() {
    this->addToIpcQueue([this]() -> bool {
        size_t size;
        bool b = TriPlayer::getSubQueueSize(size);
        if (b) {
            // Update queue if size changes
            if (this->subQueueSize_ != size) {
                this->sendGetSubQueue();
            }
            this->subQueueSize_ = size;
        }
        return b;
    });
}

void Sysmodule::sendAddToSubQueue(const SongID id) {
    this->addToIpcQueue([id]() -> bool {
        return TriPlayer::addToSubQueue(id);
    });
}

void Sysmodule::sendRemoveFromSubQueue(const size_t pos) {
    this->addToIpcQueue([pos]() -> bool {
        return TriPlayer::removeFromSubQueue(pos);
    });
}

void Sysmodule::sendSkipSubQueueSongs(const size_t n) {
    this->addToIpcQueue([n]() -> bool {
        return TriPlayer::skipSubQueueSongs(n);
    });
}

void Sysmodule::sendGetQueue() {
    this->addToIpcQueue([this]() -> bool {
        std::vector<SongID> ids;
        bool b = TriPlayer::getQueue(ids);
        if (b) {
            std::scoped_lock<std::mutex> mtx(this->queueMutex);
            this->queue_ = ids;
            this->queueChanged_ = true;
        }
        return b;
    });
}

void Sysmodule::sendGetQueueSize() {
    this->addToIpcQueue([this]() -> bool {
        size_t size;
        bool b = TriPlayer::getQueueSize(size);
        if (b) {
            // Update queue if size changes
            if (this->queueSize_ != size) {
                this->sendGetQueue();
            }
            this->queueSize_ = size;
        }
        return b;
    });
}

void Sysmodule::sendSetQueue(const std::vector<SongID> & q) {
    // Don't send empty queues
    if (q.size() == 0 || this->limit_ == 0) {
        return;
    }

    this->addToIpcQueue([q]() -> bool {
        return TriPlayer::setQueue(q);
    });
}

void Sysmodule::sendGetSongIdx() {
    this->addToIpcQueue([this]() -> bool {
        size_t idx;
        bool b = TriPlayer::getQueueIdx(idx);
        if (b) {
            // Update queues if size changes
            if (this->songIdx_ != idx) {
                this->sendGetQueue();
                this->sendGetSubQueue();
            }
            this->songIdx_ = idx;
        }
        return b;
    });
}

void Sysmodule::sendSetSongIdx(const size_t id) {
    this->addToIpcQueue([this, id]() -> bool {
        bool b = TriPlayer::setQueueIdx(id);
        if (b) {
            this->songIdx_ = id;
        }
        return b;
    });
}

void Sysmodule::sendRemoveFromQueue(const size_t pos) {
    this->addToIpcQueue([pos]() -> bool {
        return TriPlayer::removeFromQueue(pos);
    });
}

void Sysmodule::sendGetRepeat() {
    this->addToIpcQueue([this]() -> bool {
        TriPlayer::Repeat r;
        bool b = TriPlayer::getRepeatMode(r);
        if (b) {
            switch (r) {
                case TriPlayer::Repeat::Off:
                    this->repeatMode_ = RepeatMode::Off;
                    break;

                case TriPlayer::Repeat::One:
                    this->repeatMode_ = RepeatMode::One;
                    break;

                case TriPlayer::Repeat::All:
                    this->repeatMode_ = RepeatMode::All;
                    break;
            }
        }
        return b;
    });
}

void Sysmodule::sendSetRepeat(const RepeatMode m) {
    this->addToIpcQueue([this, m]() -> bool {
        TriPlayer::Repeat r = TriPlayer::Repeat::Off;
        switch (m) {
            case RepeatMode::Off:
                r = TriPlayer::Repeat::Off;
                break;

            case RepeatMode::One:
                r = TriPlayer::Repeat::One;
                break;

            case RepeatMode::All:
                r = TriPlayer::Repeat::All;
                break;
        }
        bool b = TriPlayer::setRepeatMode(r);
        if (b) {
            this->repeatMode_ = m;
        }
        return b;
    });
}

void Sysmodule::sendGetShuffle() {
    this->addToIpcQueue([this]() -> bool {
        TriPlayer::Shuffle s;
        bool b = TriPlayer::getShuffleMode(s);
        if (b) {
            this->shuffleMode_ = (s == TriPlayer::Shuffle::Off ? ShuffleMode::Off : ShuffleMode::On);
        }
        return b;
    });
}

void Sysmodule::sendSetShuffle(const ShuffleMode m) {
    this->addToIpcQueue([this, m]() -> bool {
        TriPlayer::Shuffle s = (m == ShuffleMode::Off ? TriPlayer::Shuffle::Off : TriPlayer::Shuffle::On);
        bool b = TriPlayer::setShuffleMode(s);
        if (b) {
            // Get queue on change
            this->sendGetQueue();
            this->shuffleMode_ = m;
        }
        return b;
    });
}

void Sysmodule::sendGetSong() {
    this->addToIpcQueue([this]() -> bool {
        SongID id;
        bool b = TriPlayer::getSongID(id);
        if (b) {
            this->currentSong_ = id;
        }
        return b;
    });
}

void Sysmodule::sendGetStatus() {
    this->addToIpcQueue([this]() -> bool {
        TriPlayer::Status s;
        bool b = TriPlayer::getStatus(s);
        if (b) {
            switch (s) {
                case TriPlayer::Status::Error:
                    this->status_ = PlaybackStatus::Error;
                    break;

                case TriPlayer::Status::Playing:
                    this->status_ = PlaybackStatus::Playing;
                    break;

                case TriPlayer::Status::Paused:
                    this->status_ = PlaybackStatus::Paused;
                    break;

                case TriPlayer::Status::Stopped:
                    this->status_ = PlaybackStatus::Stopped;
                    break;
            }
        }
        return b;
    });
}

void Sysmodule::sendGetPosition() {
    this->addToIpcQueue([this]() -> bool {
        double pos;
        bool b = TriPlayer::getPosition(pos);
        if (b && !this->keepPosition) {
            this->position_ = pos;
        }
        return b;
    });
}

void Sysmodule::sendSetPosition(double pos) {
    this->keepPosition = true;
    this->position_ = pos;
    this->addToIpcQueue([this, pos]() -> bool {
        bool b = TriPlayer::setPosition(pos);
        this->keepPosition = false;
        return b;
    });
}

void Sysmodule::sendGetPlayingFrom() {
    this->addToIpcQueue([this]() -> bool {
        std::string text;
        bool b = TriPlayer::getPlayingFromText(text);
        if (b) {
            std::scoped_lock<std::mutex> mtx(this->playingFromMutex);
            this->playingFrom_ = text;
        }
        return b;
    });
}

void Sysmodule::sendSetPlayingFrom(const std::string & str) {
    this->addToIpcQueue([this, str]() -> bool {
        bool b = TriPlayer::setPlayingFromText(str);
        if (b) {
            std::scoped_lock<std::mutex> mtx(this->playingFromMutex);
            this->playingFrom_ = str;
        }
        return b;
    });
}

void Sysmodule::sendReleaseDBLock() {
    this->addToIpcQueue([]() -> bool {
        return TriPlayer::releaseDatabaseLock();
    });
}

void Sysmodule::sendReloadConfig() {
    this->addToIpcQueue([]() -> bool {
        return TriPlayer::reloadConfig();
    });
}

void Sysmodule::exit() {
    this->exit_ = true;
}

Sysmodule::~Sysmodule() {
    if (this->connected_) {
        TriPlayer::exit();
    }
}