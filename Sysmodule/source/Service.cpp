#include <cstdlib>
#include <cstring>
#include "MP3.hpp"
#include "Protocol.hpp"
#include "Service.hpp"
#include "Socket.hpp"

// Port to listen on
#define LISTEN_PORT 3333
// Number of seconds to wait before previous becomes (back to start)
#define PREV_WAIT 2

MainService::MainService() {
    this->audio = Audio::getInstance();
    this->pressTime = std::time(nullptr);
    this->queue = new PlayQueue();
    this->repeatMode = RepeatMode::Off;
    this->source = nullptr;
    this->skip = false;

    // Create listening socket (exit if error occurred)
    this->socket = new Socket(3333);
    this->exit_ = !this->socket->ready();

    // Create database
    if (!this->exit_) {
        this->db = new Database();
        this->exit_ = !this->db->ready();
    }
}

void MainService::exit() {
    this->exit_ = true;
}

void MainService::process() {
    while (!this->exit_) {
        // Blocks until a message is received
        std::string msg = this->socket->readMessage();
        if (msg.length() > 0) {
            // If the message is not blank we've received data!
            size_t argIdx;
            Protocol::Command cmd = (Protocol::Command)std::stoi(msg, &argIdx);

            // Skip over separating character
            argIdx++;
            if (argIdx < msg.length()) {
                msg = msg.substr(argIdx);
            }

            std::string reply;
            switch (cmd) {
                case Protocol::Command::Version:
                    // Reply with version of protocol (sysmodule version is irrelevant)
                    reply = std::to_string(Protocol::Version);
                    break;

                case Protocol::Command::Resume:
                    this->audio->resume();
                    reply = std::to_string(this->queue->currentID());
                    break;

                case Protocol::Command::Pause:
                    this->audio->pause();
                    reply = std::to_string(this->queue->currentID());
                    break;

                case Protocol::Command::Previous:
                    // Check there is a song in the queue
                    if (this->queue->size() > 0) {
                        this->skip = true;
                        std::lock_guard<std::mutex> mtx(this->sMutex);
                        delete this->source;

                        // Check if we're at the start of the queue
                        if (this->queue->currentIdx() == 0) {
                            // Wrap around and play last song if on the first song and repeat is on
                            if (this->repeatMode == RepeatMode::All && (std::time(nullptr) - this->pressTime) >= PREV_WAIT) {
                                this->queue->setIdx(this->queue->size());
                            }

                        // Otherwise we can move backwards a song
                        } else {
                            // If pressed again go back a song
                            if ((std::time(nullptr) - this->pressTime) < PREV_WAIT) {
                                this->queue->decrementIdx();
                                this->repeatMode = (this->repeatMode != RepeatMode::Off ? RepeatMode::All : RepeatMode::Off);
                            }
                        }

                        // This will restart the current song if nothing occurred above
                        this->source = new MP3(this->db->getPathForID(this->queue->currentID()));
                        this->audio->newSong(this->source->sampleRate(), this->source->channels());

                        this->pressTime = std::time(nullptr);
                        this->skip = false;
                    }

                    reply = std::to_string(this->queue->currentID());
                    break;

                case Protocol::Command::Next: {
                    // Check there is a song in the queue
                    if (this->queue->size() > 0) {
                        this->skip = true;
                        std::lock_guard<std::mutex> mtx(this->sMutex);
                        delete this->source;

                        // Check if we're at the end of the queue
                        bool shouldStop = false;
                        if (this->queue->currentIdx() == (this->queue->size() - 1)) {
                            // Wrap around to start
                            this->queue->setIdx(0);

                            // Stop if repeat is off
                            if (this->repeatMode == RepeatMode::Off) {
                                shouldStop = true;
                            }

                        // Otherwise we're not at the end and can go forwards
                        } else {
                            this->queue->incrementIdx();
                            this->repeatMode = (this->repeatMode != RepeatMode::Off ? RepeatMode::All : RepeatMode::Off);
                        }

                        // Prepare the next song (but don't play if we wrapped around)
                        this->source = new MP3(this->db->getPathForID(this->queue->currentID()));
                        this->audio->newSong(this->source->sampleRate(), this->source->channels());

                        this->pressTime = std::time(nullptr);
                        this->skip = false;
                    }

                    reply = std::to_string(this->queue->currentID());
                    break;
                }

                case Protocol::Command::GetVolume:
                    // Round to three decimals
                    reply = std::to_string(audio->volume() + 0.005);
                    reply = reply.substr(0, reply.find(".") + 3);
                    break;

                case Protocol::Command::SetVolume: {
                    double vol = std::stod(msg);
                    this->audio->setVolume(vol);
                    reply = std::to_string(vol);
                    break;
                }

                case Protocol::Command::QueueIdx:
                    reply = std::to_string(this->queue->currentIdx());
                    break;

                case Protocol::Command::SetQueueIdx: {
                    this->queue->setIdx(std::stoi(msg));

                    // Change song
                    this->skip = true;
                    std::lock_guard<std::mutex> mtx(this->sMutex);
                    delete this->source;
                    this->source = new MP3(this->db->getPathForID(this->queue->currentID()));
                    this->audio->newSong(this->source->sampleRate(), this->source->channels());
                    this->skip = false;

                    reply = std::to_string(this->queue->currentIdx());
                    break;
                }

                case Protocol::Command::QueueSize:
                    reply = std::to_string(this->queue->size());
                    break;

                case Protocol::Command::AddToQueue: {
                    SongID id = std::stoi(msg);
                    if (!this->queue->addID(id, this->queue->size())) {
                        id = -1;    // Indicates unable to add
                    }
                    reply = std::to_string(id);
                    break;
                }

                case Protocol::Command::RemoveFromQueue: {
                    size_t pos = std::stoi(msg);
                    if (!this->queue->removeID(pos)) {
                        pos = -1;    // Indicates unable to remove
                    }
                    reply = std::to_string(pos);
                    break;
                }

                case Protocol::Command::GetQueue: {
                    if (this->queue->size() == 0) {
                        reply = std::string(1, Protocol::Delimiter);

                    } else {
                        // Get start index and number to read
                        size_t next;
                        size_t s = std::stoi(msg, &next);
                        next++;
                        msg = msg.substr(next);
                        size_t n = std::stoi(msg);
                        n = (n > this->queue->size() ? this->queue->size() : n);

                        for (size_t i = s; i < n; i++) {
                            reply += std::to_string(this->queue->IDatPosition(i));
                            if (i < n-1) {
                                reply += std::string(1, Protocol::Delimiter);
                            }
                        }

                    }
                    break;
                }

                case Protocol::Command::SetQueue: {
                    this->queue->clear();

                    // Add each token in string
                    char * str = strdup(msg.c_str());
                    msg = "";
                    char * tok = strtok(str, &Protocol::Delimiter);
                    while (tok != nullptr) {
                        this->queue->addID(strtol(tok, nullptr, 10), this->queue->size());
                        tok = strtok(nullptr, &Protocol::Delimiter);
                    }
                    free(str);

                    reply = std::to_string(this->queue->size());
                    break;
                }

                case Protocol::Command::GetRepeat: {
                    Protocol::Repeat rm = Protocol::Repeat::Off;
                    switch ((RepeatMode)std::stoi(msg)) {
                        case RepeatMode::Off:
                            rm = Protocol::Repeat::Off;
                            break;

                        case RepeatMode::One:
                            rm = Protocol::Repeat::One;
                            break;

                        case RepeatMode::All:
                            rm = Protocol::Repeat::All;
                            break;
                    }
                    reply = std::to_string((int)rm);
                    break;
                }

                case Protocol::Command::SetRepeat:
                    switch ((Protocol::Repeat)std::stoi(msg)) {
                        case Protocol::Repeat::Off:
                            this->repeatMode = RepeatMode::Off;
                            break;

                        case Protocol::Repeat::One:
                            this->repeatMode = RepeatMode::One;
                            break;

                        case Protocol::Repeat::All:
                            this->repeatMode = RepeatMode::All;
                            break;
                    }
                    reply = msg;
                    break;

                case Protocol::Command::GetShuffle:
                    reply = std::to_string((int)(this->queue->isShuffled() ? Protocol::Shuffle::On : Protocol::Shuffle::Off));
                    break;

                case Protocol::Command::SetShuffle:
                    ((Protocol::Shuffle)std::stoi(msg) == Protocol::Shuffle::Off ? this->queue->unshuffle() : this->queue->shuffle());
                    reply = std::to_string((int)(this->queue->isShuffled() ? Protocol::Shuffle::On : Protocol::Shuffle::Off));
                    break;

                case Protocol::Command::GetSong:
                    reply = std::to_string(this->queue->currentID());
                    break;

                case Protocol::Command::GetStatus: {
                    Protocol::Status s = Protocol::Status::Error;
                    switch (audio->status()) {
                        case AudioStatus::Playing:
                            s = Protocol::Status::Playing;
                            break;

                        case AudioStatus::Paused:
                            s = Protocol::Status::Paused;
                            break;

                        case AudioStatus::Stopped:
                            s = Protocol::Status::Stopped;
                            break;
                    }
                    reply = std::to_string((int)s);
                    break;
                }

                case Protocol::Command::GetPosition: {
                    if (this->source == nullptr) {
                        reply = std::to_string(0.0);
                        break;
                    }

                    // Return position to 5 digits
                    double pos = 100 * (this->audio->samplesPlayed()/(double)this->source->totalSamples());
                    reply = std::to_string(pos + 0.00005);
                    reply = reply.substr(0, reply.find(".") + 5);
                    break;
                }

                case Protocol::Command::SetPosition: {
                    this->skip = true;
                    std::lock_guard<std::mutex> mtx(this->sMutex);
                    if (this->source != nullptr) {
                        this->audio->stop();
                        this->source->seek((std::stod(msg)/100.0) * this->source->totalSamples());
                        this->audio->setSamplesPlayed(this->source->tell());
                        this->skip = false;

                        // Return position to 5 digits
                        double pos = 100 * (this->audio->samplesPlayed()/(double)this->source->totalSamples());
                        reply = std::to_string(pos + 0.00005);
                        reply = reply.substr(0, reply.find(".") + 5);

                    } else {
                        reply = std::to_string(0);
                    }
                    break;
                }

                case Protocol::Command::Reset:
                    // Disconnect from database so it can update (this will be improved)
                    delete this->db;
                    std::this_thread::sleep_for(std::chrono::seconds(Protocol::Timeout - 1));
                    this->db = new Database();
                    reply = std::to_string(Protocol::Version);
                    break;
            }

            // Send reply
            this->socket->writeMessage(reply);
        }
    }
}

void MainService::decodeSource() {
    while (!this->exit_) {
        std::lock_guard<std::mutex> mtx(this->sMutex);
        if (this->source != nullptr) {
            if (this->source->valid()) {
                // Decode into buffer
                u8 * buf = new u8[this->audio->bufferSize()];
                size_t dec = this->source->decode(buf, this->audio->bufferSize());

                // Wait until a buffer is available to queue
                while (!this->audio->bufferAvailable() && !this->skip && !this->exit_) {
                    svcSleepThread(2E+7);
                }
                if (!this->skip && !this->exit_) {
                    this->audio->addBuffer(buf, dec);
                }
                delete[] buf;
            } else {
                // Sleep for 0.1 sec if no source to play
                svcSleepThread(1E+8);
            }
        } else {
            // Sleep for 0.1 sec if no source to play
            svcSleepThread(1E+8);
        }
    }
}

MainService::~MainService() {
    delete this->db;
    delete this->queue;
    delete this->socket;
    delete this->source;
}