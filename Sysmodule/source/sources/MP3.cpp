#include <cstring>
#include "Log.hpp"
#include "MP3.hpp"

bool MP3::initLib() {
    // Initialize library
    int result = mpg123_init();
    if (result != MPG123_OK) {
        Log::writeError("[MP3] Failed to initialize library");
        return false;
    }

    Log::writeSuccess("[MP3] Initialized successfully");
    return true;
}

MP3::MP3(std::string path) : Source() {
    Log::writeInfo("[MP3] Opening file: " + path);

    // Create instance
    int result;
    this->mpg = mpg123_new(nullptr, &result);
    if (this->mpg == nullptr) {
        Log::writeError("[MP3] Failed to create instance");
        this->valid_ = false;
    }

    if (this->valid_) {
        // Enable gapless decoding
        result = mpg123_param(this->mpg, MPG123_FLAGS, MPG123_GAPLESS, 0.0);
        if (result != MPG123_OK) {
            Log::writeWarning("[MP3] Unable to enable gapless decoding");
        }

        // Attempt to open file
        int result = mpg123_open(this->mpg, path.c_str());
        if (result != MPG123_OK) {
            Log::writeError("[MP3] Unable to open file");
            this->valid_ = false;
        }
    }

    // Get format
    if (this->valid_) {
        int encoding;
        result = mpg123_getformat(this->mpg, &this->sampleRate_, &this->channels_, &encoding);
        if (result != MPG123_OK) {
            Log::writeError("[MP3] Unable to get format from file");
            this->valid_ = false;
        }
    }

    // Get length
    if (this->valid_) {
        this->totalSamples_ = mpg123_length(this->mpg);
        if (this->totalSamples_ == MPG123_ERR) {
            Log::writeWarning("[MP3] Unable to determine length of song");
            this->totalSamples_ = 1;
        }
    }

    if (this->valid_) {
        Log::writeSuccess("[MP3] File opened successfully");
    }
}

size_t MP3::decode(unsigned char * buf, size_t sz) {
    if (!this->valid_) {
        return 0;
    }

    size_t decoded = 0;
    std::memset(buf, 0, sz);
    mpg123_read(mpg, buf, sz, &decoded);
    if (decoded == 0) {
        Log::writeWarning("[MP3] Finished decoding file");
        this->valid_ = false;
    } else {
        Log::writeInfo("[MP3] Decoded " + std::to_string(decoded) + " bytes");
    }

    this->decodedSamples_ += decoded;
    return decoded;
}

MP3::~MP3() {
    if (this->mpg != nullptr) {
        mpg123_close(this->mpg);
        mpg123_delete(this->mpg);
    }
}