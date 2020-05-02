#include <cstdlib>
#include "Protocol.hpp"
#include "Service.hpp"
#include "Socket.hpp"

// Port to listen on
#define LISTEN_PORT 3333

MainService::MainService() {
    this->audio = Audio::getInstance();
    this->currentID = -1;
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
                    reply = std::to_string(this->currentID);
                    break;

                case Protocol::Command::Pause:
                    this->audio->pause();
                    reply = std::to_string(this->currentID);
                    break;

                case Protocol::Command::Previous:
                    // Change song here!
                    reply = std::to_string(this->currentID);
                    break;

                case Protocol::Command::Next:
                    // Change song here!
                    reply = std::to_string(this->currentID);
                    break;

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

                case Protocol::Command::Play: {
                    this->currentID = std::stoi(msg);
                    std::string path = this->db->getPathForID(this->currentID);

                    // Play song if path returned
                    if (path.length() > 0) {
                        this->skip = true;
                        std::lock_guard<std::mutex> mtx(this->sMutex);
                        // Create new source
                        delete this->source;
                        this->source = new MP3(path);

                        // Prepare Audio class for new format
                        this->audio->newSong(this->source->sampleRate(), this->source->channels());
                        this->skip = false;
                        reply = std::to_string(this->currentID);
                    } else {
                        reply = std::to_string(-1);
                    }
                    break;
                }

                case Protocol::Command::QueueIdx:
                    // Query queue pos here!
                    reply = std::to_string(0);
                    break;

                case Protocol::Command::AddToQueue:
                    // Add to queue here!
                    reply = std::to_string(0);
                    break;

                case Protocol::Command::RemoveFromQueue:
                    // Remove from queue here!
                    reply = std::to_string(0);
                    break;

                case Protocol::Command::GetQueue:
                    // Concat queue
                    reply = std::to_string(0);
                    break;

                case Protocol::Command::SetQueue:
                    // Return length of queue
                    reply = std::to_string(0);
                    break;

                case Protocol::Command::GetRepeat:
                    // Return repeat
                    reply = std::to_string(0);
                    break;

                case Protocol::Command::SetRepeat:
                    // Adjust repeat here
                    reply = std::to_string(0);
                    break;

                case Protocol::Command::GetShuffle:
                    // Return shuffle
                    reply = std::to_string(0);
                    break;

                case Protocol::Command::SetShuffle:
                    // Adjust shuffle here
                    reply = std::to_string(0);
                    break;

                case Protocol::Command::GetSong:
                    reply = std::to_string(this->currentID);
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
                while (!this->audio->bufferAvailable() && !this->skip) {
                    svcSleepThread(2E+7);
                }
                if (!this->skip) {
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
    delete this->socket;
    delete this->source;
}