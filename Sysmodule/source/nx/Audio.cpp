#include <cstdlib>
#include <cstring>
#include "Log.hpp"
#include "nx/Audio.hpp"
#include "nx/NX.hpp"
#include <switch.h>

constexpr size_t bufferSize = 0x3C00;       // Size of each buffer (15kB)
constexpr size_t maxBuffers = 4;            // Maximum number of buffer slots
constexpr size_t outputChannels = 2;        // Number of channels to output (should always be 2)

Audio * Audio::instance = nullptr;          // Our singleton instance
static AudioDriver drv;                     // Audio output driver (should be safe as this is a singleton class)

// Real size of a buffer due to alignment
constexpr size_t realSize = ((bufferSize + (AUDREN_MEMPOOL_ALIGNMENT - 1)) &~ (AUDREN_MEMPOOL_ALIGNMENT - 1));

Audio::Audio() {
    this->nextBuf = 0;
    this->waveBuf = nullptr;
    this->exit_ = true;
    this->memPool = nullptr;
    this->sampleOffset = 0;
    this->sink = -1;
    this->status_ = Status::Stopped;
    this->success = true;
    this->voice = -1;
    this->vol = 100.0;

    // Create the driver
    constexpr AudioRendererConfig audrenCfg = {
        .output_rate     = AudioRendererOutputRate_48kHz,
        .num_voices      = 4,
        .num_effects     = 0,
        .num_sinks       = 1,
        .num_mix_objs    = 1,
        .num_mix_buffers = 2,
    };
    Result rc = audrvCreate(&drv, &audrenCfg, outputChannels);
    if (R_FAILED(rc)) {
        this->success = false;
        Log::writeError("[AUDIO] Unable to create driver!");
    }

    // Create wave buffers
    this->waveBuf = new AudioDriverWaveBuf[maxBuffers];
    if (this->waveBuf == nullptr) {
        delete[] this->waveBuf;
        this->success = false;
        Log::writeError("[AUDIO] Unable to allocate memory for buffers!");
    }

    // Allocate memory pool and align
    if (this->success) {
        this->memPool = new uint8_t *[maxBuffers];
        bool error = false;
        for (size_t i = 0; i < maxBuffers; i++) {
            this->memPool[i] = static_cast<uint8_t *>(aligned_alloc(AUDREN_MEMPOOL_ALIGNMENT, realSize));
            if (this->memPool[i] == nullptr) {
                error = true;
                break;
            }
        }

        if (error) {
            this->success = false;
            for (size_t i = 0; i < maxBuffers; i++) {
                free(this->memPool[i]);
            }
            delete[] this->memPool;
            delete[] this->waveBuf;
            audrvClose(&drv);
            Log::writeError("[AUDIO] Unable to allocate memory pool (size: " + std::to_string(maxBuffers) + "x" + std::to_string(realSize) + ")");
        }
    }

    // Register memory pools with driver and set sink
    if (this->success) {
        for (size_t i = 0; i < maxBuffers; i++) {
            int id = audrvMemPoolAdd(&drv, this->memPool[i], realSize);
            audrvMemPoolAttach(&drv, id);
        }
        const uint8_t sinkChannels[outputChannels] = {0, 1};
        this->sink = audrvDeviceSinkAdd(&drv, AUDREN_DEFAULT_DEVICE_NAME, 2, sinkChannels);
        audrvUpdate(&drv);
        this->exit_ = false;
        Log::writeSuccess("[AUDIO] Audio object created successfully");
    }
}

Audio * Audio::getInstance() {
    if (Audio::instance == nullptr) {
        Audio::instance = new Audio();
    }
    return Audio::instance;
}

bool Audio::initialized() {
    return this->success;
}

void Audio::exit() {
    this->exit_ = true;
}

void Audio::newSong(long rate, int channels) {
    this->stop();
    std::scoped_lock<std::mutex> mtx(this->mutex);
    this->sampleOffset = 0;

    // Drop previous voice
    if (this->voice >= 0) {
        audrvVoiceDrop(&drv, this->voice);
        audrvUpdate(&drv);
        this->voice = -1;
    }

    // Create voice matching rate and channels
    this->channels = channels;
    this->voice = 0;
    bool b = audrvVoiceInit(&drv, this->voice, this->channels, PcmFormat_Int16, rate);
    if (!b) {
        this->voice = -1;
        Log::writeError("[AUDIO] Failed to init a new voice!");
    } else {
        // Set volume levels
        audrvVoiceSetDestinationMix(&drv, this->voice, AUDREN_FINAL_MIX_ID);
        if (this->channels == 1) {
            // Mono audio
            audrvVoiceSetMixFactor(&drv, this->voice, 1.0f, 0, 0);
            audrvVoiceSetMixFactor(&drv, this->voice, 1.0f, 0, 1);
        } else {
            // Stereo-o-o
            audrvVoiceSetMixFactor(&drv, this->voice, 1.0f, 0, 0);
            audrvVoiceSetMixFactor(&drv, this->voice, 0.0f, 0, 1);
            audrvVoiceSetMixFactor(&drv, this->voice, 0.0f, 1, 0);
            audrvVoiceSetMixFactor(&drv, this->voice, 1.0f, 1, 1);
        }
        Log::writeInfo("[AUDIO] Created a new voice");
    }
    Log::writeInfo("[AUDIO] Rate: " + std::to_string(rate) +  ", Channels: " + std::to_string(channels));
}

void Audio::addBuffer(uint8_t * buf, size_t sz) {
    // Ensure appropriate size and a buffer is available
    if (sz > realSize || sz == 0 || !this->bufferAvailable()) {
        return;
    }

    // Copy contents into mempool
    std::scoped_lock<std::mutex> mtx(this->mutex);
    std::memcpy(this->memPool[this->nextBuf], buf, sz);
    armDCacheFlush(this->memPool[this->nextBuf], sz);

    // Fill relevant waveBuf
    this->waveBuf[this->nextBuf].data_raw = this->memPool[nextBuf];
    this->waveBuf[this->nextBuf].size = sz;
    this->waveBuf[this->nextBuf].start_sample_offset = 0;
    this->waveBuf[this->nextBuf].end_sample_offset = sz/(2 * this->channels);
    audrvVoiceAddWaveBuf(&drv, this->voice, &this->waveBuf[this->nextBuf]);

    // Move to next buffer
    this->nextBuf = (this->nextBuf + 1) % maxBuffers;

    // Indicate playing
    if (this->status_ == Status::Stopped) {
        audrvVoiceStart(&drv, this->voice);
        this->status_ = Status::Playing;
    }
}

bool Audio::bufferAvailable() {
    std::scoped_lock<std::mutex> mtx(this->mutex);
    return (this->waveBuf[this->nextBuf].state == AudioDriverWaveBufState_Done);
}

size_t Audio::bufferSize() {
    return realSize;
}

void Audio::resume() {
    if (this->status_ == Status::Paused) {
        std::scoped_lock<std::mutex> mtx(this->mutex);
        audrvVoiceSetPaused(&drv, this->voice, false);
        audrvUpdate(&drv);
        this->status_ = Status::Playing;
    }
}

void Audio::pause() {
    if (this->status_ == Status::Playing) {
        std::scoped_lock<std::mutex> mtx(this->mutex);
        audrvVoiceSetPaused(&drv, this->voice, true);
        audrvUpdate(&drv);
        this->status_ = Status::Paused;
    }
}

void Audio::stop() {
    std::scoped_lock<std::mutex> mtx(this->mutex);
    if (this->voice >= 0) {
        this->sampleOffset += audrvVoiceGetPlayedSampleCount(&drv, this->voice);
        audrvVoiceStop(&drv, this->voice);
        audrvUpdate(&drv);
    }

    // Indicate buffers are 'empty'
    for (size_t i = 0; i < maxBuffers; i++) {
        this->waveBuf[i].state = AudioDriverWaveBufState_Done;
    }
    this->nextBuf = 0;
    this->status_ = Status::Stopped;
}

Audio::Status Audio::status() {
    return this->status_;
}

int Audio::samplesPlayed() {
    if (this->voice < 0) {
        return this->sampleOffset;
    }

    std::scoped_lock<std::mutex> mtx(this->mutex);
    return (this->sampleOffset + audrvVoiceGetPlayedSampleCount(&drv, this->voice));
}

void Audio::setSamplesPlayed(int s) {
    std::scoped_lock<std::mutex> mtx(this->mutex);
    this->sampleOffset = s;
}

double Audio::volume() {
    return this->vol;
}

void Audio::setVolume(double v) {
    // Check it's safe to change
    if (v < 0.0d || v > 100.0d || !this->success) {
        return;
    }

    std::scoped_lock<std::mutex> mtx(this->mutex);
    this->vol = v;
    audrvMixSetVolume(&drv, this->sink, this->vol/100.0);
    Log::writeInfo("[AUDIO] Volume set to " + std::to_string(this->vol));
}

void Audio::process() {
    while (!this->exit_) {
        switch (this->status_) {
            case Status::Playing: {
                std::unique_lock<std::mutex> mtx(this->mutex);
                audrvUpdate(&drv);
                audrenWaitFrame();

                // Check if we need to move to stopped state (no more buffers)
                int lastBuf = ((this->nextBuf - 1) < 0 ? maxBuffers-1 : this->nextBuf - 1);
                if (this->waveBuf[lastBuf].state == AudioDriverWaveBufState_Done) {
                    mtx.unlock();
                    this->stop();
                }
                break;
            }

            case Status::Paused:
            case Status::Stopped:
                // Sleep if not doing anything
                NX::Thread::sleepMilli(5);
                break;
        }
    }
}

Audio::~Audio() {
    if (this->success) {
        // Drop voice
        if (this->voice >= 0) {
            audrvVoiceStop(&drv, this->voice);
            audrvVoiceDrop(&drv, this->voice);
        }

        // Free stuff
        for (size_t i = 0; i < maxBuffers; i++) {
            free(this->memPool[i]);
        }
        delete[] this->memPool;
        audrvClose(&drv);
        delete[] this->waveBuf;
    }
    Audio::instance = nullptr;
}