#include "Config.hpp"
#include "Database.hpp"
#include "ipc/TriPlayer.hpp"
#include "nx/Audio.hpp"
#include "nx/NX.hpp"
#include "Paths.hpp"
#include "PlayQueue.hpp"
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

    // Create ipc server
    this->ipcServer = new Ipc::Server("tri", 3);
    this->ipcServer->setRequestHandler([this](Ipc::Request * r) -> uint32_t {
        return static_cast<uint32_t>(this->commandThread(r));
    });

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

Ipc::Result MainService::commandThread(Ipc::Request * request) {
    switch (static_cast<Ipc::Command>(request->cmd())) {
        case Ipc::Command::Version:
            request->appendReplyValue(std::string(VER_STRING));
            break;

        case Ipc::Command::Resume: {
            this->audio->resume();
            break;
        }

        case Ipc::Command::Pause: {
            this->audio->pause();
            break;
        }

        // Simply set the 'SongAction' to Previous/Replay based on time of last press
        // The other thread will handle changing songs
        case Ipc::Command::Previous:
            // Change song if within timeframe
            if ((std::time(nullptr) - this->pressTime) < PREV_WAIT) {
                this->songAction = SongAction::Previous;

            // Otherwise restart the current song
            } else {
                this->songAction = SongAction::Replay;
            }

            this->pressTime = std::time(nullptr);
            break;

        // Simply set the 'SongAction' to Next
        // The other thread will handle changing songs
        case Ipc::Command::Next:
            this->songAction = SongAction::Next;
            this->pressTime = std::time(nullptr);
            break;

        case Ipc::Command::GetVolume:
            request->appendReplyValue(this->audio->volume());
            break;

        case Ipc::Command::SetVolume: {
            double vol;
            Ipc::Result rc = request->readRequestValue(vol);
            if (rc != Ipc::Result::Ok || vol < 0.0d || vol > 100.0d) {
                return Ipc::Result::BadInput;
            }
            this->audio->setVolume(vol);
            break;
        }

        case Ipc::Command::Mute: {
            double vol = this->audio->volume();
            if (vol > 0.0) {
                this->muteLevel = vol;
                this->audio->setVolume(0);
            }
            break;
        }

        case Ipc::Command::Unmute: {
            if (this->muteLevel > 0.0) {
                this->audio->setVolume(this->muteLevel);
                this->muteLevel = 0.0;
            }
            double vol = this->audio->volume();
            request->appendReplyValue(vol);
            break;
        }

        case Ipc::Command::GetSubQueue: {
            // Return if empty
            std::unique_lock<std::shared_mutex> mtx(this->sqMutex);
            if (this->subQueue.empty()) {
                size_t zero = 0;
                request->appendReplyValue(zero);
                break;
            }

            // Read first arg (index of first song to get)
            size_t index;
            Ipc::Result rc = request->readRequestValue(index);
            if (rc != Ipc::Result::Ok) {
                return rc;
            }

            // Read second arg (number to get)
            size_t count;
            rc = request->readRequestValue(count);
            if (rc != Ipc::Result::Ok) {
                return rc;
            }

            // Return if requesting zero
            if (count == 0) {
                request->appendReplyValue(count);
                break;
            }

            // Iterate over sub queue and append each ID
            size_t max = (count > this->subQueue.size()-index ? this->subQueue.size()-index : count);
            while (index < max) {
                request->appendReplyData(this->subQueue[index]);
                index++;
            }
            request->appendReplyValue(max);
            break;
        }

        case Ipc::Command::SkipSubQueueSongs: {
            // Get argument (number to skip)
            size_t count;
            Ipc::Result rc = request->readRequestValue(count);
            if (rc != Ipc::Result::Ok) {
                return rc;
            }

            // Pop songs from queue and skip
            size_t skipped = 0;
            std::unique_lock<std::shared_mutex> mtx(this->sqMutex);
            while (skipped < count && !this->subQueue.empty()) {
                this->subQueue.pop_front();
                skipped++;
            }
            this->songAction = SongAction::Next;
            request->appendReplyValue(skipped);
            break;
        }

        case Ipc::Command::SubQueueSize: {
            std::shared_lock<std::shared_mutex> mtx(this->sqMutex);
            request->appendReplyValue(this->subQueue.size());
            break;
        }

        case Ipc::Command::AddToSubQueue: {
            // Read song id from args
            SongID id;
            Ipc::Result rc = request->readRequestValue(id);
            if (rc != Ipc::Result::Ok) {
                return rc;
            }

            // Lock and update queue
            std::unique_lock<std::shared_mutex> mtx(this->sqMutex);
            if (this->subQueue.size() < SUBQUEUE_MAX_SIZE) {
                this->subQueue.push_back(id);
                mtx.unlock();

                // Start playing if there is nothing playing
                std::shared_lock<std::shared_mutex> qMtx(this->qMutex);
                if (this->queue->currentID() == -1) {
                    this->songAction = SongAction::Next;
                }

            // Return error code if subqueue full
            } else {
                return Ipc::Result::SubQueueFull;
            }
            break;
        }

        case Ipc::Command::RemoveFromSubQueue: {
            // Read index from args
            size_t index;
            Ipc::Result rc = request->readRequestValue(index);
            if (rc != Ipc::Result::Ok) {
                return rc;
            }

            // Erase element
            std::unique_lock<std::shared_mutex> mtx(this->sqMutex);
            index = (index >= this->subQueue.size() ? this->subQueue.size()-1 : index);
            this->subQueue.erase(this->subQueue.begin() + index);
            break;
        }

        case Ipc::Command::QueueIdx: {
            std::shared_lock<std::shared_mutex> mtx(this->qMutex);
            request->appendReplyValue(this->queue->currentIdx());
            break;
        }

        case Ipc::Command::SetQueueIdx: {
            // Get position to jump to from args
            size_t pos;
            Ipc::Result rc = request->readRequestValue(pos);
            if (rc != Ipc::Result::Ok) {
                return rc;
            }

            // Jump to and return current index
            std::unique_lock<std::shared_mutex> mtx(this->qMutex);
            this->queue->setIdx(pos);
            this->songAction = SongAction::Replay;
            request->appendReplyValue(this->queue->currentIdx());
            break;
        }

        case Ipc::Command::QueueSize: {
            std::shared_lock<std::shared_mutex> mtx(this->qMutex);
            request->appendReplyValue(this->queue->size());
            break;
        }

        case Ipc::Command::RemoveFromQueue: {
            // Get position to remove from args
            size_t pos;
            Ipc::Result rc = request->readRequestValue(pos);
            if (rc != Ipc::Result::Ok) {
                return rc;
            }

            // Remove from queue
            std::unique_lock<std::shared_mutex> mtx(this->qMutex);
            if (!this->queue->removeID(pos)) {
                return Ipc::Result::BadInput;
            }
            break;
        }

        case Ipc::Command::GetQueue: {
            // Return if empty
            std::unique_lock<std::shared_mutex> mtx(this->qMutex);
            if (this->queue->empty()) {
                size_t zero = 0;
                request->appendReplyValue(zero);
                break;
            }

            // Read first arg (index of first song to get)
            size_t index;
            Ipc::Result rc = request->readRequestValue(index);
            if (rc != Ipc::Result::Ok) {
                return rc;
            }

            // Read second arg (number to get)
            size_t count;
            rc = request->readRequestValue(count);
            if (rc != Ipc::Result::Ok) {
                return rc;
            }

            // Return if requesting zero
            if (count == 0) {
                request->appendReplyValue(count);
                break;
            }

            // Iterate over queue and append each ID
            size_t max = (count > this->queue->size()-index ? this->queue->size()-index : count);
            while (index < max) {
                request->appendReplyData(this->queue->IDatPosition(index));
                index++;
            }
            request->appendReplyValue(max);
            break;
        }

        case Ipc::Command::SetQueue: {
            // Clear sub queue
            std::unique_lock<std::shared_mutex> sqMtx(this->sqMutex);
            this->subQueue.clear();
            sqMtx.unlock();

            // Clear main queue
            std::unique_lock<std::shared_mutex> mtx(this->qMutex);
            this->queue->clear();

            // Add each value present in the buffer
            while (true) {
                SongID id;
                Ipc::Result rc = request->readRequestData(id);
                if (rc != Ipc::Result::Ok) {
                    break;
                }
                this->queue->addID(id, this->queue->size());
            }

            // Reply with number of songs inserted
            request->appendReplyValue(this->queue->size());
            break;
        }

        case Ipc::Command::GetRepeat: {
            TriPlayer::Repeat rm = TriPlayer::Repeat::Off;
            switch (this->repeatMode) {
                case RepeatMode::Off:
                    rm = TriPlayer::Repeat::Off;
                    break;

                case RepeatMode::One:
                    rm = TriPlayer::Repeat::One;
                    break;

                case RepeatMode::All:
                    rm = TriPlayer::Repeat::All;
                    break;
            }
            request->appendReplyValue(rm);
            break;
        }

        case Ipc::Command::SetRepeat: {
            // Read repeat mode from args
            TriPlayer::Repeat rm;
            Ipc::Result rc = request->readRequestValue(rm);
            if (rc != Ipc::Result::Ok) {
                return rc;
            }

            // Set matching mode
            switch (rm) {
                case TriPlayer::Repeat::Off:
                    this->repeatMode = RepeatMode::Off;
                    break;

                case TriPlayer::Repeat::One:
                    this->repeatMode = RepeatMode::One;
                    break;

                case TriPlayer::Repeat::All:
                    this->repeatMode = RepeatMode::All;
                    break;
            }
            break;
        }

        case Ipc::Command::GetShuffle: {
            std::shared_lock<std::shared_mutex> mtx(this->qMutex);
            request->appendReplyValue((this->queue->isShuffled() ? TriPlayer::Shuffle::On : TriPlayer::Shuffle::Off));
            break;
        }

        case Ipc::Command::SetShuffle: {
            // Read shuffle mode from args
            TriPlayer::Shuffle sm;
            Ipc::Result rc = request->readRequestValue(sm);
            if (rc != Ipc::Result::Ok) {
                return rc;
            }

            // Adjust accordingly
            std::unique_lock<std::shared_mutex> mtx(this->qMutex);
            if (sm == TriPlayer::Shuffle::Off) {
                this->queue->unshuffle();
            } else {
                this->queue->shuffle();
            }
            break;
        }

        case Ipc::Command::GetSong: {
            std::shared_lock<std::shared_mutex> mtx(this->qMutex);
            request->appendReplyValue(this->queue->currentID());
            break;
        }

        case Ipc::Command::GetStatus: {
            TriPlayer::Status s = TriPlayer::Status::Error;

            // Say that we're playing if the song is currently seeking
            if (this->seekTo >= 0) {
                s = TriPlayer::Status::Playing;
            } else {
                switch (audio->status()) {
                    case Audio::Status::Playing:
                        s = TriPlayer::Status::Playing;
                        break;

                    case Audio::Status::Paused:
                        s = TriPlayer::Status::Paused;
                        break;

                    case Audio::Status::Stopped:
                        s = TriPlayer::Status::Stopped;
                        break;
                }
            }
            request->appendReplyValue(s);
            break;
        }

        case Ipc::Command::GetPosition: {
            // Check position if not seeking
            double pos = 100.0 * this->seekTo;
            if (pos < 0) {
                std::shared_lock<std::shared_mutex> mtx(this->sMutex);
                if (this->source == nullptr) {
                    pos = 0;
                } else {
                    pos = 100 * (this->audio->samplesPlayed()/(double)this->source->totalSamples());
                }
            }
            request->appendReplyValue(pos);
            break;
        }

        case Ipc::Command::SetPosition: {
            // Read position from args
            double pos;
            Ipc::Result rc = request->readRequestValue(pos);
            if (rc != Ipc::Result::Ok) {
                return rc;
            }

            // Set seek value and return it
            pos /= 100.0;
            this->seekTo = pos;
            request->appendReplyValue(pos);
            break;
        }

        case Ipc::Command::GetPlayingFrom: {
            std::shared_lock<std::shared_mutex> mtx(this->qMutex);
            request->appendReplyData(this->playingFrom);
            break;
        }

        case Ipc::Command::SetPlayingFrom: {
            // Read string from input buffer
            std::string str;
            Ipc::Result rc = request->readRequestData(str);
            if (rc != Ipc::Result::Ok) {
                return rc;
            }

            // Lock queue to allow updating and return string
            std::unique_lock<std::shared_mutex> mtx(this->qMutex);
            this->playingFrom = str.substr(0, (str.length() > 100) ? 100 : str.length());
            request->appendReplyData(this->playingFrom);
            break;
        }

        // Lock the mutex and mark that the database is being used for writing by the app
        // Once we lock the mutex the decode thread is guaranteed to not be using the DB
        case Ipc::Command::RequestDBLock: {
            std::scoped_lock<std::mutex> mtx(this->dbMutex);
            this->db->close();
            this->dbLocked = true;
            break;
        }

        // Mark the database as unlocked
        case Ipc::Command::ReleaseDBLock:
            this->dbLocked = false;
            break;

        case Ipc::Command::ReloadConfig:
            this->updateConfig();
            break;

        case Ipc::Command::Reset: {
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

            request->appendReplyValue(std::string(VER_STRING));
            break;
        }

        case Ipc::Command::Quit:
            this->exit_ = true;
            break;
    }

    // If we make it this far then everything went OK
    return Ipc::Result::Ok;
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

void MainService::ipcThread() {
    while (!this->exit_) {
        // Stop the service if a fatal error occurs
        if (!this->ipcServer->process()) {
            this->exit();
        }
    }
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

MainService::~MainService() {
    delete this->cfg;
    delete this->db;
    delete this->ipcServer;
    delete this->queue;
    delete this->source;
}