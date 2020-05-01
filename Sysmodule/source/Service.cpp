#include <cstdlib>
#include "Commands.h"
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
            SM_Command cmd = (SM_Command)std::stoi(msg, &argIdx);

            // Skip over separating character
            argIdx++;
            if (argIdx < msg.length()) {
                msg = msg.substr(argIdx);
            }

            std::string reply = "";
            switch (cmd) {
                case VERSION:
                    // Reply with version of protocol (sysmodule version is irrelevant)
                    reply = std::to_string(SM_PROTOCOL_VERSION);
                    break;

                case RESUME:
                    this->audio->resume();
                    break;

                case PAUSE:
                    this->audio->pause();
                    break;

                case PREVIOUS:
                    //
                    break;

                case NEXT:
                    //
                    break;

                case GETVOLUME:
                    // Round to three decimals
                    reply = std::to_string(audio->volume() + 0.005);
                    reply = reply.substr(0, reply.find(".") + 3);
                    break;

                case SETVOLUME:
                    this->audio->setVolume(std::stod(msg));
                    break;

                case PLAY: {
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
                    }
                    break;
                }

                case ADDTOQUEUE:
                    //
                    break;

                case REMOVEFROMQUEUE:
                    //
                    break;

                case GETQUEUE:
                    //
                    break;

                case SETQUEUE:
                    //
                    break;

                case SHUFFLE:
                    //
                    break;

                case SETREPEAT:
                    //
                    break;

                case GETSONG:
                    reply = std::to_string(this->currentID);
                    break;

                case GETSTATUS: {
                    SM_Status s = ERROR;
                    switch (audio->status()) {
                        case AudioStatus::Playing:
                            s = PLAYING;
                            break;

                        case AudioStatus::Paused:
                            s = PAUSED;
                            break;

                        case AudioStatus::Stopped:
                            s = STOPPED;
                            break;
                    }
                    reply = std::to_string((int)s);
                    break;
                }

                case GETPOSITION: {
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

                case GETSHUFFLE:
                    //
                    break;

                case GETREPEAT:
                    //
                    break;

                case RESET:
                    // Disconnect from database so it can update (this will be improved)
                    delete this->db;
                    std::this_thread::sleep_for(std::chrono::seconds(10));
                    this->db = new Database();
                    break;
            }

            // Send reply if there is one
            if (reply.length() > 0) {
                this->socket->writeMessage(reply);
            }
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