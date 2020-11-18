#define DR_FLAC_IMPLEMENTATION
#include "decoders/dr_flac.h"

#include <cstring>
#include "Log.hpp"
#include "source/FLAC.hpp"

// Inherit actual struct
struct dr_flac : public drflac {};

namespace Source {
    FLAC::FLAC(const std::string & path) : Source() {
        // Create decoder for file
        Log::writeInfo("[FLAC] Opening file: " + path);
        this->flac = static_cast<dr_flac *>(drflac_open_file(path.c_str(), nullptr));

        // Check if opened succesfully
        if (this->flac == nullptr) {
            Log::writeError("[FLAC] Unable to open file");
            this->valid_ = false;
            return;
        }

        // Get all required output data
        this->channels_ = this->flac->channels;
        this->sampleRate_ = this->flac->sampleRate;
        this->totalSamples_ = this->flac->totalPCMFrameCount;

        Log::writeInfo("[FLAC] File opened successfully");
    }

    size_t FLAC::decode(unsigned char * buf, size_t sz) {
        if (!this->valid_) {
            return 0;
        }

        const size_t frames = sz/this->channels_/sizeof(int16_t);
        std::memset(buf, 0, sz);
        size_t decoded = drflac_read_pcm_frames_s16(this->flac, frames, reinterpret_cast<int16_t *>(buf));
        if (decoded == 0) {
            Log::writeInfo("[FLAC] Finished decoding file");
            this->done_ = true;
        }

        return (decoded * this->channels_ * sizeof(int16_t));
    }

    void FLAC::seek(size_t pos) {
        if (!this->valid_) {
            return;
        }

        drflac_bool32 ok = drflac_seek_to_pcm_frame(this->flac, pos);
        if (ok != DRFLAC_TRUE) {
            Log::writeError("[FLAC] An error occurred attempting to seek to: " + std::to_string(pos));
        }
    }

    size_t FLAC::tell() {
        if (!this->valid_) {
            return 0;
        }

        return this->flac->currentPCMFrame;
    }

    FLAC::~FLAC() {
        drflac_close(this->flac);
    }
};