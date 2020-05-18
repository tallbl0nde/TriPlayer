#include "Audio.hpp"
#include <cstring>
#include <malloc.h>
#include "Log.hpp"

// Number of buffers
#define BUFFER_COUNT 5
// Size of a single buffer (in bytes)
#define BUFFER_SIZE 0xA000  // 40kB
// Number of audio channels to output (shouldn't need to change)
#define OUT_CHANNELS 2
// Real size of a buffer
#define REAL_SIZE ((BUFFER_SIZE + (AUDREN_MEMPOOL_ALIGNMENT - 1)) &~ (AUDREN_MEMPOOL_ALIGNMENT - 1))

Audio * Audio::instance = nullptr;

Audio::Audio() {
    this->nextBuf = 0;
    this->waveBuf = nullptr;
    this->exit_ = true;
    this->memPool = nullptr;
    this->sampleOffset = 0;
    this->sink = -1;
    this->status_ = AudioStatus::Stopped;
    this->success = true;
    this->voice = -1;
    this->vol = 100.0;

    // Create the driver
    Result rc = audrvCreate(&this->drv, &audrenCfg, OUT_CHANNELS);
    if (R_FAILED(rc)) {
        this->success = false;
        Log::writeError("[AUDIO] Unable to create driver!");
    }

    // Create wave buffers
    this->waveBuf = new AudioDriverWaveBuf[BUFFER_COUNT];
    if (this->waveBuf == nullptr) {
        delete[] this->waveBuf;
        this->success = false;
        Log::writeError("[AUDIO] Unable to allocate memory for buffers!");
    }

    // Allocate memory pool and align
    if (this->success) {
        this->memPool = new u8 *[BUFFER_COUNT];
        bool error = false;
        for (int i = 0; i < BUFFER_COUNT; i++) {
            this->memPool[i] = (u8 *) memalign(AUDREN_MEMPOOL_ALIGNMENT, REAL_SIZE);
            if (this->memPool[i] == nullptr) {
                error = true;
                break;
            }
        }

        if (error) {
            this->success = false;
            for (int i = 0; i < BUFFER_COUNT; i++) {
                free(this->memPool[i]);
            }
            delete[] this->memPool;
            delete[] this->waveBuf;
            audrvClose(&this->drv);
            Log::writeError("[AUDIO] Unable to allocate memory pool (size: " + std::to_string(BUFFER_COUNT) + "x" + std::to_string(REAL_SIZE) + ")");
        }
    }

    // Register memory pools with driver and set sink
    if (this->success) {
        for (int i = 0; i < BUFFER_COUNT; i++) {
            int id = audrvMemPoolAdd(&this->drv, this->memPool[i], REAL_SIZE);
            audrvMemPoolAttach(&this->drv, id);
        }
        const u8 sinkChannels[OUT_CHANNELS] = {0, 1};
        this->sink = audrvDeviceSinkAdd(&this->drv, AUDREN_DEFAULT_DEVICE_NAME, 2, sinkChannels);
        audrvUpdate(&this->drv);
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
    this->sampleOffset = 0;

    std::lock_guard<std::mutex> mtx(this->mutex);
    // Drop previous voice
    if (this->voice >= 0) {
        audrvVoiceDrop(&this->drv, this->voice);
        audrvUpdate(&this->drv);
        this->voice = -1;
    }

    // Create voice matching rate and channels
    this->channels = channels;
    this->voice = 0;
    bool b = audrvVoiceInit(&this->drv, this->voice, this->channels, PcmFormat_Int16, rate);
    if (!b) {
        this->voice = -1;
        Log::writeError("[AUDIO] Failed to init a new voice!");
    } else {
        // Set volume levels
        audrvVoiceSetDestinationMix(&this->drv, this->voice, AUDREN_FINAL_MIX_ID);
        if (this->channels == 1) {
            // Mono audio
            audrvVoiceSetMixFactor(&this->drv, this->voice, 1.0f, 0, 0);
            audrvVoiceSetMixFactor(&this->drv, this->voice, 1.0f, 0, 1);
        } else {
            // Stereo-o-o
            audrvVoiceSetMixFactor(&this->drv, this->voice, 1.0f, 0, 0);
            audrvVoiceSetMixFactor(&this->drv, this->voice, 0.0f, 0, 1);
            audrvVoiceSetMixFactor(&this->drv, this->voice, 0.0f, 1, 0);
            audrvVoiceSetMixFactor(&this->drv, this->voice, 1.0f, 1, 1);
        }
        Log::writeSuccess("[AUDIO] Created a new voice");
    }
    Log::writeInfo("[AUDIO] Rate: " + std::to_string(rate) +  ", Channels: " + std::to_string(channels));
}

void Audio::addBuffer(u8 * buf, size_t sz) {
    // Ensure below allocated size
    if (sz > REAL_SIZE || sz == 0 || !this->bufferAvailable()) {
        return;
    }

    std::lock_guard<std::mutex> mtx(this->mutex);
    // Copy contents into mempool
    armDCacheFlush(this->memPool[this->nextBuf], sz);
    std::memcpy(this->memPool[this->nextBuf], buf, sz);

    // Fill relevant waveBuf
    this->waveBuf[this->nextBuf].data_raw = this->memPool[nextBuf];
    this->waveBuf[this->nextBuf].size = sz;
    this->waveBuf[this->nextBuf].start_sample_offset = 0;
    this->waveBuf[this->nextBuf].end_sample_offset = sz/(2 * this->channels);
    audrvVoiceAddWaveBuf(&this->drv, this->voice, &this->waveBuf[this->nextBuf]);

    // Move to next buffer
    this->nextBuf = (this->nextBuf + 1) % BUFFER_COUNT;

    // Indicate playing
    if (this->status_ == AudioStatus::Stopped) {
        audrvVoiceStart(&this->drv, this->voice);
        this->status_ = AudioStatus::Playing;
    }
}

bool Audio::bufferAvailable() {
    std::lock_guard<std::mutex> mtx(this->mutex);
    return (this->waveBuf[this->nextBuf].state == AudioDriverWaveBufState_Done);
}

size_t Audio::bufferSize() {
    return REAL_SIZE;
}

void Audio::resume() {
    if (this->status_ == AudioStatus::Paused) {
        std::lock_guard<std::mutex> mtx(this->mutex);
        audrvVoiceSetPaused(&this->drv, this->voice, false);
        audrvUpdate(&this->drv);
        this->status_ = AudioStatus::Playing;
    }
}

void Audio::pause() {
    if (this->status_ == AudioStatus::Playing) {
        std::lock_guard<std::mutex> mtx(this->mutex);
        audrvVoiceSetPaused(&this->drv, this->voice, true);
        audrvUpdate(&this->drv);
        this->status_ = AudioStatus::Paused;
    }
}

void Audio::stop() {
    std::lock_guard<std::mutex> mtx(this->mutex);
    if (this->voice >= 0) {
        this->sampleOffset += audrvVoiceGetPlayedSampleCount(&this->drv, this->voice);
        audrvVoiceStop(&this->drv, this->voice);
        audrvUpdate(&this->drv);
    }

    // Indicate buffers are 'empty'
    for (int i = 0; i < BUFFER_COUNT; i++) {
        this->waveBuf[i].state = AudioDriverWaveBufState_Done;
    }
    this->nextBuf = 0;
    this->status_ = AudioStatus::Stopped;
}

AudioStatus Audio::status() {
    return this->status_;
}

int Audio::samplesPlayed() {
    if (this->voice < 0) {
        return this->sampleOffset;
    }

    std::lock_guard<std::mutex> mtx(this->mutex);
    return (this->sampleOffset + audrvVoiceGetPlayedSampleCount(&this->drv, this->voice));
}

void Audio::setSamplesPlayed(int s) {
    std::lock_guard<std::mutex> mtx(this->mutex);
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

    std::lock_guard<std::mutex> mtx(this->mutex);
    this->vol = v;
    audrvMixSetVolume(&this->drv, this->sink, this->vol/100.0);
    Log::writeInfo("[AUDIO] Volume set to " + std::to_string(this->vol));
}

void Audio::process() {
    while (!this->exit_) {
        switch (this->status_) {
            case AudioStatus::Playing: {
                std::unique_lock<std::mutex> mtx(this->mutex);
                audrvUpdate(&this->drv);
                audrenWaitFrame();
                mtx.unlock();

                // Check if we need to move to stopped state (no more buffers)
                int lastBuf = ((nextBuf - 1) < 0 ? BUFFER_COUNT-1 : nextBuf - 1);
                if (waveBuf[lastBuf].state == AudioDriverWaveBufState_Done) {
                    this->stop();
                }
                break;
            }

            case AudioStatus::Paused:
            case AudioStatus::Stopped:
                // Sleep if not doing anything for 0.02 sec
                svcSleepThread(2E+7);
                break;
        }
    }
}

Audio::~Audio() {
    if (this->success) {
        // Drop voice
        if (this->voice >= 0) {
            audrvVoiceStop(&this->drv, this->voice);
            audrvVoiceDrop(&this->drv, this->voice);
        }

        // Free stuff
        for (int i = 0; i < BUFFER_COUNT; i++) {
            free(this->memPool[i]);
        }
        delete[] this->memPool;
        audrvClose(&this->drv);
        delete[] this->waveBuf;
    }
    Audio::instance = nullptr;
}