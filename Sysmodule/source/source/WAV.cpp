#define DR_WAV_IMPLEMENTATION
#include "decoders/dr_wav.h"

#include <cstring>
#include "Log.hpp"
#include "source/WAV.hpp"
#include "Types.hpp"

// Inherit actual struct
struct dr_wav : public drwav {};

namespace Source {
    WAV::WAV(const std::string & path) : Source() {
        // Create decoder for file
        Log::writeInfo("[WAV] Opening file: " + path);
        this->wav = new dr_wav;
        drwav_bool32 ok = drwav_init_file(static_cast<drwav *>(this->wav), path.c_str(), nullptr);
        if (ok != DRWAV_TRUE) {
            Log::writeError("[WAV] Unable to open file");
            this->valid_ = false;
            return;
        }

        // Get all required output data
        this->channels_ = this->wav->channels;
        this->sampleRate_ = this->wav->sampleRate;
        this->totalSamples_ = this->wav->totalPCMFrameCount;

        this->frameSize = drwav_get_bytes_per_pcm_frame(this->wav);

        // This is set to Int16 as libnx doesn't support anything else,
        // but should it support other bit depths this class is ready for it!
        this->format_ = Format::Int16;
        // this->format_ = static_cast<Format>(this->wav->bitsPerSample/8);

        Log::writeInfo("[WAV] File opened successfully");
    }

    size_t WAV::decode(unsigned char * buf, size_t sz) {
        if (!this->valid_) {
            return 0;
        }

        // Call appropriate decode function based on bit depth of samples
        // Sample size is rounded up to preserve quality (e.g. 24 bits -> 32 bits)
        size_t decoded = 0;
        std::memset(buf, 0, sz);
        const size_t frames = sz/this->channels_/(static_cast<int>(this->format_));
        switch (this->format_) {
            case Format::Int8:
            case Format::Int16:
                decoded = drwav_read_pcm_frames_s16(this->wav, frames, reinterpret_cast<int16_t *>(buf));
                break;

            case Format::Int24:
            case Format::Int32:
                decoded = drwav_read_pcm_frames_s32(this->wav, frames, reinterpret_cast<int32_t *>(buf));
                break;

            // This actually shouldn't be called
            case Format::Float:
                decoded = drwav_read_pcm_frames_f32(this->wav, frames, reinterpret_cast<float *>(buf));
                break;
        }
        if (decoded == 0) {
            Log::writeInfo("[WAV] Finished decoding file");
            this->done_ = true;
        }

        return (decoded * this->channels_ * (static_cast<int>(this->format_)));
    }

    void WAV::seek(size_t pos) {
        if (!this->valid_) {
            return;
        }

        drwav_bool32 ok = drwav_seek_to_pcm_frame(this->wav, pos);
        if (ok != DRWAV_TRUE) {
            Log::writeError("[WAV] An error occurred attempting to seek to: " + std::to_string(pos));
        }
    }

    size_t WAV::tell() {
        if (!this->valid_) {
            return 0;
        }

        // Determine which PCM frame we're currently in (get bytes processed
        // so far and divide by the size of one PCM frame)
        size_t pos = this->wav->dataChunkDataSize - this->wav->bytesRemaining;
        return (pos / this->frameSize);
    }

    WAV::~WAV() {
        drwav_uninit(this->wav);
        delete this->wav;
    }
};