#include <cstdlib>
#include <cstring>
#include <future>
#include "NX.hpp"
#include "Paths.hpp"
#include "Service.hpp"
#include "sources/MP3.hpp"
#include "utils/FS.hpp"

// Interval (in seconds) to test if DB file is accessible
#define DB_TEST_INTERVAL 2
// Number of milliseconds between polling system state
#define POLL_INTERVAL 10
// Number of seconds to wait before previous becomes (back to start)
#define PREV_WAIT 2
// Max size of sub-queue (requires 20kB)
#define SUBQUEUE_MAX_SIZE 5000

MainService::MainService() {
    this->audio = Audio::getInstance();
    this->dbLocked = false;
    this->muteLevel = 0.0;
    this->pressTime = std::time(nullptr);
    this->queue = new PlayQueue();
    this->repeatMode = RepeatMode::Off;
    this->seekTo = -1;
    this->source = nullptr;
    this->songAction = SongAction::Nothing;

    // Read and set config
    this->cfg = new Config(Path::Sys::ConfigFile);
    this->updateConfig();

    // Create listening socket (exit if error occurred)
    this->listener = new Socket::Listener(Protocol::Port);
    this->exit_ = !this->listener->isListening();

    // Create database
    if (!this->exit_) {
        this->db = new Database();
    } else {
        this->db = nullptr;
    }
}

void MainService::updateConfig() {
    Log::setLogLevel(this->cfg->logLevel());

    std::scoped_lock<std::shared_mutex> sMtx(this->sMutex);
    MP3::setAccurateSeek(this->cfg->MP3AccurateSeek());
    MP3::setEqualizer(this->cfg->MP3Equalizer());
}

void MainService::commandThread(Socket::Transfer * socket) {
    // Loop until we've been signalled to exit or the socket loses connection
    while (!this->exit_ && socket->isConnected()) {
        // Wait for a message
        std::string msg = socket->readMessage();
        if (msg.length() == 0) {
            continue;
        }

        // Extract command and args from message
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

            case Protocol::Command::Previous:
                // Simply set the 'SongAction' to Previous/Replay based on time of last press
                // The other thread will handle changing songs

                // Change song if within timeframe
                if ((std::time(nullptr) - this->pressTime) < PREV_WAIT) {
                    this->songAction = SongAction::Previous;

                // Otherwise restart the current song
                } else {
                    this->songAction = SongAction::Replay;
                }

                this->pressTime = std::time(nullptr);
                reply = "0";
                break;

            case Protocol::Command::Next:
                // Simply set the 'SongAction' to Next
                // The other thread will handle changing songs
                this->songAction = SongAction::Next;
                this->pressTime = std::time(nullptr);
                reply = "0";
                break;

            case Protocol::Command::GetVolume:
                // Round to three decimals
                reply = std::to_string(this->audio->volume() + 0.005);
                reply = reply.substr(0, reply.find(".") + 3);
                break;

            case Protocol::Command::SetVolume: {
                double vol = std::stod(msg);
                this->audio->setVolume(vol);
                reply = std::to_string(vol);
                break;
            }

            case Protocol::Command::Mute: {
                double vol = this->audio->volume();
                if (vol > 0.0) {
                    this->muteLevel = vol;
                    this->audio->setVolume(0);
                }
                reply = "0";
                break;
            }

            case Protocol::Command::Unmute:
                if (this->muteLevel > 0.0) {
                    this->audio->setVolume(this->muteLevel);
                    this->muteLevel = 0.0;
                }

                // Round to three decimals
                reply = std::to_string(this->audio->volume() + 0.005);
                reply = reply.substr(0, reply.find(".") + 3);
                break;

            case Protocol::Command::GetSubQueue: {
                std::shared_lock<std::shared_mutex> mtx(this->sqMutex);
                if (this->subQueue.empty()) {
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
                this->songAction = SongAction::Next;
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
                    mtx.unlock();
                    reply = std::to_string(id);

                    // Start playing if there is nothing playing
                    std::shared_lock<std::shared_mutex> qMtx(this->qMutex);
                    if (this->queue->currentID() == -1) {
                        this->songAction = SongAction::Next;
                    }
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
                std::shared_lock<std::shared_mutex> mtx(this->qMutex);
                this->queue->setIdx(std::stoi(msg));
                this->songAction = SongAction::Replay;
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
                if (this->queue->empty()) {
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
                std::unique_lock<std::shared_mutex> sqMtx(this->sqMutex);
                this->subQueue.clear();
                sqMtx.unlock();

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

                // Say that we're playing if the song is currently seeking
                if (this->seekTo >= 0) {
                    s = Protocol::Status::Playing;
                } else {
                    switch (audio->status()) {
                        case Audio::Status::Playing:
                            s = Protocol::Status::Playing;
                            break;

                        case Audio::Status::Paused:
                            s = Protocol::Status::Paused;
                            break;

                        case Audio::Status::Stopped:
                            s = Protocol::Status::Stopped;
                            break;
                    }
                }
                reply = std::to_string((int)s);
                break;
            }

            case Protocol::Command::GetPosition: {
                // Return position to 5 digits
                double pos = 100.0 * this->seekTo;
                if (pos < 0) {
                    // Check position if not seeking
                    std::shared_lock<std::shared_mutex> mtx(this->sMutex);
                    if (this->source == nullptr) {
                        reply = "0";
                        break;
                    }
                    pos = 100 * (this->audio->samplesPlayed()/(double)this->source->totalSamples());
                }
                reply = std::to_string(pos + 0.00005);
                reply = reply.substr(0, reply.find(".") + 5);
                break;
            }

            case Protocol::Command::SetPosition: {
                // Return position to 5 digits
                double pos = std::stod(msg);
                this->seekTo = pos/100.0;
                reply = std::to_string(pos + 0.00005);
                reply = reply.substr(0, reply.find(".") + 5);
                break;
            }

            case Protocol::Command::GetPlayingFrom: {
                // Replying with an empty string triggers an error
                std::shared_lock<std::shared_mutex> mtx(this->qMutex);
                reply = (this->playingFrom.empty() ? " " : this->playingFrom);
                break;
            }

            case Protocol::Command::SetPlayingFrom: {
                std::unique_lock<std::shared_mutex> mtx(this->qMutex);
                this->playingFrom = msg.substr(0, (msg.length() > 100) ? 100 : msg.length());
                reply = (this->playingFrom.empty() ? " " : this->playingFrom);
                break;
            }

            case Protocol::Command::RequestDBLock: {
                // Lock the mutex and mark that the database is being used for writing by the app
                // Once we lock the mutex the decode thread is guaranteed to not be using the DB
                std::scoped_lock<std::mutex> mtx(this->dbMutex);
                this->db->close();
                this->dbLocked = true;

                reply = std::to_string(0);
                break;
            }

            case Protocol::Command::ReleaseDBLock:
                // Mark the database as unlocked
                this->dbLocked = false;
                reply = std::to_string(0);
                break;

            case Protocol::Command::ReloadConfig:
                // This locks relevant mutexes, etc.
                this->updateConfig();
                reply = std::to_string(0);
                break;

            case Protocol::Command::Reset: {
                // Need to lock everything!!
                std::scoped_lock<std::shared_mutex> sMtx(this->sMutex);
                std::scoped_lock<std::shared_mutex> sqMtx(this->sqMutex);
                std::scoped_lock<std::shared_mutex> qMtx(this->qMutex);
                std::scoped_lock<std::mutex> mtx(this->dbMutex);

                // Ensure we're disconnected from the DB
                this->db->close();

                // Stop playback and empty queues
                this->audio->stop();
                this->queue->clear();
                this->subQueue.clear();
                delete this->source;
                this->source = nullptr;

                reply = std::to_string(Protocol::Version);
                break;
            }
        }

        // Send reply
        socket->writeMessage(reply);
    }

    // Delete the socket now that we're done
    delete socket;
}

void MainService::exit() {
    this->exit_ = true;
}

void MainService::gpioEventThread() {
    // Prepare gpio
    if (!NX::Gpio::prepare()) {
        Log::writeWarning("[GPIO] Couldn't prepare session, unable to pause when headset unplugged!");
        return;
    }

    // Loop until the service has signalled to exit
    while (!this->exit_) {
        if (NX::Gpio::headsetUnplugged()) {
            this->audio->pause();
        }
        NX::Thread::sleepMilli(POLL_INTERVAL);
    }

    // Cleanup
    NX::Gpio::cleanup();
}

void MainService::playbackThread() {
    while (!this->exit_) {
        std::unique_lock<std::shared_mutex> sMtx(this->sMutex);
        std::unique_lock<std::shared_mutex> sqMtx(this->sqMutex);
        std::unique_lock<std::shared_mutex> qMtx(this->qMutex);

        // Change source if the current song has been changed
        if (this->songAction != SongAction::Nothing) {
            // Only do something if a queue has something in it
            if (!(this->queue->empty() && this->subQueue.empty())) {
                switch (this->songAction) {
                    case SongAction::Previous:
                        // If repeat is on and we're at the start, wrap around
                        if (this->repeatMode != RepeatMode::Off && this->queue->currentIdx() == 0) {
                            this->queue->setIdx(this->queue->size());

                        // Go back to last song on queue otherwise (won't do anything if at the start)
                        } else {
                            this->queue->decrementIdx();
                        }

                        this->repeatMode = (this->repeatMode != RepeatMode::Off ? RepeatMode::All : RepeatMode::Off);
                        break;

                    case SongAction::Next:
                        // If repeat is on and we're at the end, wrap around
                        if (this->repeatMode != RepeatMode::Off && (this->queue->currentIdx() == this->queue->size() - 1) && this->subQueue.empty()) {
                            this->queue->setIdx(0);

                        // Otherwise advance to next song (check subqueue if there's one there)
                        } else {
                            // Check if we need to pop off of subqueue
                            if (!this->subQueue.empty()) {
                                this->queue->addID(this->subQueue.front(), this->queue->currentIdx() + 1);
                                this->subQueue.pop_front();
                            }

                            this->queue->incrementIdx();
                        }

                        this->repeatMode = (this->repeatMode != RepeatMode::Off ? RepeatMode::All : RepeatMode::Off);
                        break;

                    default:
                        // Do nothing if set to SongAction::Replay (just want to replay current song)
                        break;
                }

                // In order to read the file path we need to:
                // - Lock the mutex and either:
                // -> Wait until it is marked as unlocked OR
                // -> Wait until it's readable (in case application crashes)
                std::unique_lock<std::mutex> mtx(this->dbMutex);
                std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now();
                while (this->dbLocked) {
                    NX::Thread::sleepMilli(50);
                    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
                    if (std::chrono::duration_cast< std::chrono::duration<double> >(now - last).count() > DB_TEST_INTERVAL) {
                        if (Utils::Fs::fileAccessible("/switch/TriPlayer/data.sqlite3")) {
                            this->dbLocked = false;
                        }
                        last = now;
                    }
                }

                // Now that the database is available actually read from it (note that this read-only connection
                // is left intact until either RESET or REQUESTDBLOCK is received)
                if (!this->db->openReadOnly()) {
                    this->exit_ = true;
                }
                std::string path = this->db->getPathForID(this->queue->currentID());
                mtx.unlock();

                // Delete old source and prepare a new one
                delete this->source;
                this->source = new MP3(path);
                this->audio->newSong(this->source->sampleRate(), this->source->channels());
            }
            this->songAction = SongAction::Nothing;
        }

        // Don't need queues for a while
        qMtx.unlock();
        sqMtx.unlock();

        bool sleep = true;
        if (this->source != nullptr) {
            sleep = false;

            // Seek to a position if required
            if (this->source->valid() && this->seekTo >= 0) {
                this->audio->stop();
                this->source->seek(this->seekTo * this->source->totalSamples());
                this->audio->setSamplesPlayed(this->source->tell());
                this->seekTo = -1;
            }

            // If the source is not corrupt and not done decode into an available buffer
            if (this->source->valid() && !this->source->done()) {
                uint8_t * buf = new uint8_t[this->audio->bufferSize()];
                size_t dec = this->source->decode(buf, this->audio->bufferSize());
                sMtx.unlock();

                // Wait until a buffer is available to queue or there is an update
                while (this->songAction == SongAction::Nothing && this->seekTo < 0 && !this->exit_) {
                    if (this->audio->bufferAvailable()) {
                        this->audio->addBuffer(buf, dec);
                        break;

                    // Sleep if no buffer is available (duration depends on state)
                    } else {
                        NX::Thread::sleepMilli((this->audio->status() == Audio::Status::Paused ? 20 : 5));
                    }
                }
                delete[] buf;

            // Otherwise if the source is not corrupt and has finished being decoded, wait until the audio device has finished playing it's buffers
            } else if (this->source->valid() && this->source->done() && this->audio->status() != Audio::Status::Stopped) {
                sleep = true;

            // If not valid attempt to move to change song
            } else {
                sqMtx.lock();
                qMtx.lock();

                // Replay current song if repeat is set to one
                if (this->repeatMode == RepeatMode::One && this->source->valid()) {
                    this->songAction = SongAction::Replay;

                // Don't go to next song if at the end and repeat is off
                } else if (this->queue->currentIdx() >= this->queue->size() - 1 && this->repeatMode == RepeatMode::Off && this->subQueue.empty()) {
                    this->songAction = SongAction::Nothing;

                // Otherwise advance to next song
                } else {
                    this->songAction = SongAction::Next;
                }

                qMtx.unlock();
                sqMtx.unlock();

                if (this->songAction == SongAction::Nothing) {
                    sleep = true;
                }
            }
        }

        // Sleep if no action is required
        if (sleep) {
            NX::Thread::sleepMilli(50);
        }
    }
}

void MainService::sleepEventThread() {
    // Prepare psc
    if (!NX::Psc::prepare()) {
        Log::writeWarning("[PSC] Couldn't prepare session, unable to pause when entering sleep!");
        return;
    }

    // Loop until the service has signalled to exit
    while (!this->exit_) {
        if (NX::Psc::enteringSleep(POLL_INTERVAL)) {
            this->audio->pause();
        }
    }

    // Cleanup
    NX::Psc::cleanup();
}

void MainService::socketThread() {
    // Quit if not listening
    if (!this->listener->isListening()) {
        return;
    }

    // First we need to start listening for connections
    std::future<void> listeningThread = std::async(std::launch::async, [this]() {
        this->listener->listen(this->exit_);
    });

    // Spawn new threads for each connection
    std::vector< std::future<void> > threads;
    while (!this->exit_) {
        // Remove any finished threads
        size_t i = 0;
        while (i < threads.size()) {
            if (threads[i].wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                threads.erase(threads.begin() + i);
            } else {
                i++;
            }
        }

        // Check for connections and start a thread if needed
        while (this->listener->hasTransferSocket()) {
            // Start the thread (it is responsible for deleting the socket when done)
            Socket::Transfer * socket = this->listener->getTransferSocket();
            threads.push_back(std::async(std::launch::async, [this, socket]() {
                this->commandThread(socket);
            }));
        }

        // Sleep for 100ms and then check again
        NX::Thread::sleepMilli(100);
    }

    // If we've been signaled to exit wait for the listening thread to finish
    listeningThread.get();
}

MainService::~MainService() {
    delete this->cfg;
    delete this->db;
    delete this->listener;
    delete this->queue;
    delete this->source;
}