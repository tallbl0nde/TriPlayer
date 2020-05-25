#include <cstdlib>
#include <cstring>
#include "MP3.hpp"
#include "Service.hpp"

// Number of seconds to wait before previous becomes (back to start)
#define PREV_WAIT 2
// Max size of sub-queue (requires 20kB)
#define SUBQUEUE_MAX_SIZE 5000

MainService::MainService() {
    this->audio = Audio::getInstance();
    this->pressTime = std::time(nullptr);
    this->queue = new PlayQueue();
    this->repeatMode = RepeatMode::Off;
    this->seekTo = -1;
    this->source = nullptr;
    this->songChanged = false;

    // Create listening socket (exit if error occurred)
    this->socket = new Socket(Protocol::Port);
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

void MainService::audioThread() {
    while (!this->exit_) {
        std::unique_lock<std::shared_mutex> sMtx(this->sMutex);
        std::unique_lock<std::shared_mutex> sqMtx(this->sqMutex);
        std::unique_lock<std::shared_mutex> qMtx(this->qMutex);

        // Change source if the current song has been changed
        if (this->songChanged) {
            // Check if we need to pop off of subqueue
            if (!this->subQueue.empty()) {
                this->queue->addID(this->subQueue.front(), this->queue->currentIdx());
                this->subQueue.pop_front();
            }

            delete this->source;
            this->source = new MP3(this->db->getPathForID(this->queue->currentID()));
            this->audio->newSong(this->source->sampleRate(), this->source->channels());
            this->songChanged = false;
        }
        qMtx.unlock();
        sqMtx.unlock();

        // Seek if required
        if (this->seekTo >= 0 && this->source != nullptr) {
            this->audio->stop();
            this->source->seek(this->seekTo * this->source->totalSamples());
            this->audio->setSamplesPlayed(this->source->tell());
            this->seekTo = -1;
        }

        // Decode/change
        if (this->source != nullptr) {
            if (this->source->valid() && !this->source->done()) {
                // Decode into buffer
                u8 * buf = new u8[this->audio->bufferSize()];
                size_t dec = this->source->decode(buf, this->audio->bufferSize());
                sMtx.unlock();

                // Wait until a buffer is available to queue or there is an update
                while (!this->audio->bufferAvailable() && !(this->songChanged || this->seekTo >= 0) && !this->exit_) {
                    svcSleepThread(2E+7);
                }
                if (!(this->songChanged || this->seekTo >= 0) && !this->exit_) {
                    this->audio->addBuffer(buf, dec);
                }
                delete[] buf;

            // If not valid attempt to move to next song in queue
            } else {
                // Check if there is another song to play
                sqMtx.lock();
                qMtx.lock();
                if (this->queue->currentIdx() >= this->queue->size() - 1) {
                    if (this->repeatMode == RepeatMode::One && this->source->valid()) {
                        this->songChanged = true;
                    } else if (this->repeatMode == RepeatMode::All) {
                        this->queue->setIdx(0);
                        this->songChanged = true;
                    }
                } else {
                    if (this->repeatMode == RepeatMode::One && this->source->valid()) {
                        this->songChanged = true;
                    } else {
                        this->queue->incrementIdx();
                        this->songChanged = true;
                    }
                }
                qMtx.unlock();
                sqMtx.unlock();

                // Otherwise just sleep until a new song is chosen
                if (!this->songChanged) {
                    svcSleepThread(1E+8);
                }
            }

        } else {
            // Sleep for 0.1 sec if no source to play
            svcSleepThread(1E+8);
        }
    }
}

void MainService::socketThread() {
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

                case Protocol::Command::Resume: {
                    this->audio->resume();
                    std::shared_lock<std::shared_mutex> mtx(this->qMutex);
                    reply = std::to_string(this->queue->currentID());
                    break;
                }

                case Protocol::Command::Pause: {
                    this->audio->pause();
                    std::shared_lock<std::shared_mutex> mtx(this->qMutex);
                    reply = std::to_string(this->queue->currentID());
                    break;
                }

                case Protocol::Command::Previous: {
                    // Check there is a song in the queue
                    std::unique_lock<std::shared_mutex> mtx(this->qMutex);
                    if (this->queue->size() > 0) {
                        // Check if we're at the start of the queue
                        if (this->queue->currentIdx() == 0) {
                            // Wrap around and play last song if on the first song and repeat is on
                            if ((std::time(nullptr) - this->pressTime) < PREV_WAIT) {
                                if (this->repeatMode != RepeatMode::Off) {
                                    this->queue->setIdx(this->queue->size());
                                    this->repeatMode = (this->repeatMode != RepeatMode::Off ? RepeatMode::All : RepeatMode::Off);
                                }
                            }

                        // Otherwise we can move backwards a song
                        } else {
                            if ((std::time(nullptr) - this->pressTime) < PREV_WAIT) {
                                this->queue->decrementIdx();
                                this->repeatMode = (this->repeatMode != RepeatMode::Off ? RepeatMode::All : RepeatMode::Off);
                            }
                        }

                        this->pressTime = std::time(nullptr);
                        this->songChanged = true;   // Always (re)start song when previous is pressed
                    }

                    reply = std::to_string(this->queue->currentID());
                    break;
                }

                case Protocol::Command::Next: {
                    // Check there is a song in the queue
                    std::unique_lock<std::shared_mutex> mtx(this->qMutex);
                    if (this->queue->size() > 0) {
                        // Check if we're at the end of the queue
                        if (this->queue->currentIdx() == (this->queue->size() - 1)) {
                            // Wrap around to start
                            this->queue->setIdx(0);
                            this->repeatMode = (this->repeatMode != RepeatMode::Off ? RepeatMode::All : RepeatMode::Off);

                        // Otherwise we're not at the end and can go forwards
                        } else {
                            this->queue->incrementIdx();
                            this->repeatMode = (this->repeatMode != RepeatMode::Off ? RepeatMode::All : RepeatMode::Off);
                        }

                        this->pressTime = std::time(nullptr);
                        this->songChanged = true;   // Always (re)start song when next is pressed
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

                case Protocol::Command::GetSubQueue: {
                    std::shared_lock<std::shared_mutex> mtx(this->sqMutex);
                    if (this->subQueue.size() == 0) {
                        reply = std::string(1, Protocol::Delimiter);

                    } else {
                        // Get start index and number to read
                        size_t next;
                        size_t s = std::stoi(msg, &next);
                        next++;
                        msg = msg.substr(next);
                        size_t n = std::stoi(msg);

                        // Return nothing if requesting zero!
                        if (n == 0) {
                            reply = std::string(1, Protocol::Delimiter);
                        } else {
                            n = (n > this->subQueue.size() ? this->subQueue.size() : n);
                            for (size_t i = s; i < n; i++) {
                                reply += std::to_string(this->subQueue[i]);
                                if (i < n-1) {
                                    reply += std::string(1, Protocol::Delimiter);
                                }
                            }
                        }
                    }
                    break;
                }

                case Protocol::Command::SkipSubQueueSongs: {
                    size_t j = std::stoi(msg);
                    size_t i;
                    std::unique_lock<std::shared_mutex> mtx(this->sqMutex);
                    for (i = 0; i < j; i++) {
                        this->subQueue.pop_front();
                    }
                    this->songChanged = true;
                    reply = std::to_string(i);
                    break;
                }

                case Protocol::Command::SubQueueSize: {
                    std::shared_lock<std::shared_mutex> mtx(this->sqMutex);
                    reply = std::to_string(this->subQueue.size());
                    break;
                }

                case Protocol::Command::AddToSubQueue: {
                    SongID id = std::stoi(msg);
                    std::unique_lock<std::shared_mutex> mtx(this->sqMutex);
                    if (this->subQueue.size() < SUBQUEUE_MAX_SIZE) {
                        this->subQueue.push_back(id);
                        reply = std::to_string(id);
                    } else {
                        reply = std::to_string(-1);
                    }
                    break;
                }

                case Protocol::Command::RemoveFromSubQueue: {
                    size_t pos = std::stoi(msg);
                    std::unique_lock<std::shared_mutex> mtx(this->sqMutex);
                    pos = (pos > this->subQueue.size() ? this->subQueue.size()-1 : pos);
                    this->subQueue.erase(this->subQueue.begin() + pos);
                    reply = std::to_string(pos);
                    break;
                }

                case Protocol::Command::QueueIdx: {
                    std::shared_lock<std::shared_mutex> mtx(this->qMutex);
                    reply = std::to_string(this->queue->currentIdx());
                    break;
                }

                case Protocol::Command::SetQueueIdx: {
                    std::unique_lock<std::shared_mutex> mtx(this->qMutex);
                    this->queue->setIdx(std::stoi(msg));
                    this->songChanged = true;
                    reply = std::to_string(this->queue->currentIdx());
                    break;
                }

                case Protocol::Command::QueueSize: {
                    std::shared_lock<std::shared_mutex> mtx(this->qMutex);
                    reply = std::to_string(this->queue->size());
                    break;
                }

                case Protocol::Command::RemoveFromQueue: {
                    size_t pos = std::stoi(msg);
                    std::unique_lock<std::shared_mutex> mtx(this->qMutex);
                    if (!this->queue->removeID(pos)) {
                        pos = -1;    // Indicates unable to remove
                    }
                    reply = std::to_string(pos);
                    break;
                }

                case Protocol::Command::GetQueue: {
                    std::shared_lock<std::shared_mutex> mtx(this->qMutex);
                    if (this->queue->size() == 0) {
                        reply = std::string(1, Protocol::Delimiter);

                    } else {
                        // Get start index and number to read
                        size_t next;
                        size_t s = std::stoi(msg, &next);
                        next++;
                        msg = msg.substr(next);
                        size_t n = std::stoi(msg);

                        // Return nothing if requesting zero!
                        if (n == 0) {
                            reply = std::string(1, Protocol::Delimiter);
                        } else {
                            n = (n > this->queue->size() ? this->queue->size() : n);
                            for (size_t i = s; i < n; i++) {
                                reply += std::to_string(this->queue->IDatPosition(i));
                                if (i < n-1) {
                                    reply += std::string(1, Protocol::Delimiter);
                                }
                            }
                        }
                    }
                    break;
                }

                case Protocol::Command::SetQueue: {
                    std::unique_lock<std::shared_mutex> mtx(this->qMutex);
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
                    switch (this->repeatMode) {
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

                case Protocol::Command::GetShuffle: {
                    std::shared_lock<std::shared_mutex> mtx(this->qMutex);
                    reply = std::to_string((int)(this->queue->isShuffled() ? Protocol::Shuffle::On : Protocol::Shuffle::Off));
                    break;
                }

                case Protocol::Command::SetShuffle: {
                    std::unique_lock<std::shared_mutex> mtx(this->qMutex);
                    ((Protocol::Shuffle)std::stoi(msg) == Protocol::Shuffle::Off ? this->queue->unshuffle() : this->queue->shuffle());
                    reply = std::to_string((int)(this->queue->isShuffled() ? Protocol::Shuffle::On : Protocol::Shuffle::Off));
                    break;
                }

                case Protocol::Command::GetSong: {
                    std::shared_lock<std::shared_mutex> mtx(this->qMutex);
                    reply = std::to_string(this->queue->currentID());
                    break;
                }

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
                    std::shared_lock<std::shared_mutex> mtx(this->sMutex);
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
                    this->seekTo = std::stod(msg)/100.0;

                    // Return position to 5 digits
                    std::shared_lock<std::shared_mutex> mtx(this->sMutex);
                    if (this->source == nullptr) {
                        reply = std::to_string(0.0);
                        break;
                    }

                    double pos = 100 * (this->audio->samplesPlayed()/(double)this->source->totalSamples());
                    reply = std::to_string(pos + 0.00005);
                    reply = reply.substr(0, reply.find(".") + 5);
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

MainService::~MainService() {
    delete this->db;
    delete this->queue;
    delete this->socket;
    delete this->source;
}