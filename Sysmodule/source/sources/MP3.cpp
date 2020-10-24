#include <cstring>
#include "Log.hpp"
#include "nx/File.hpp"
#include <mpg123.h>
#include "sources/MP3.hpp"

mpg123_handle * MP3::mpg = nullptr;

MP3::MP3(const std::string & path) : Source() {
    Log::writeInfo("[MP3] Opening file: " + path);

    // Check the library is initialized
    if (this->mpg == nullptr) {
        Log::writeError("[MP3] Couldn't open file as library is not initialized!");
        this->valid_ = false;
        return;
    }

    // Attempt to open file
    this->file = new NX::File(path);
    int result = mpg123_open_handle(this->mpg, this->file);
    if (result != MPG123_OK) {
        MP3::logErrorMsg();
        Log::writeError("[MP3] Unable to open file");
        this->valid_ = false;
        return;
    }

    // Get format
    int encoding;
    result = mpg123_getformat(this->mpg, &this->sampleRate_, &this->channels_, &encoding);
    if (result != MPG123_OK) {
        MP3::logErrorMsg();
        Log::writeError("[MP3] Unable to get format from file");
        this->valid_ = false;
        return;
    }

    // Get length
    this->totalSamples_ = mpg123_length(this->mpg);
    if (this->totalSamples_ == MPG123_ERR) {
        Log::writeWarning("[MP3] Unable to determine length of song");
        this->totalSamples_ = 1;
    }

    Log::writeInfo("[MP3] File opened successfully");
}

void MP3::logErrorMsg() {
    const char * msg = mpg123_strerror(MP3::mpg);
    std::string str(msg);
    Log::writeError("[MP3] " + str);
}

size_t MP3::decode(unsigned char * buf, size_t sz) {
    if (!this->valid_) {
        return 0;
    }

    size_t decoded = 0;
    std::memset(buf, 0, sz);
    mpg123_read(mpg, buf, sz, &decoded);
    if (decoded == 0) {
        Log::writeInfo("[MP3] Finished decoding file");
        this->done_ = true;
    }

    return decoded;
}

void MP3::seek(size_t pos) {
    if (!this->valid_) {
        return;
    }

    off_t res = mpg123_seek(this->mpg, pos, SEEK_SET);
    if (res < 0) {
        Log::writeError("[MP3] An error occurred attempting to seek to: " + std::to_string(pos));
    }
}

size_t MP3::tell() {
    if (!this->valid_) {
        return 0;
    }

    int pos = mpg123_tell(this->mpg);
    if (pos == MPG123_ERR) {
        return 0;
    }
    return pos;
}

MP3::~MP3() {
    if (this->mpg != nullptr) {
        mpg123_close(this->mpg);
    }

    // Delete file handle
    delete this->file;
}

bool MP3::initLib() {
    // Initialize library
    int result = mpg123_init();
    if (result != MPG123_OK) {
        Log::writeError("[MP3] Failed to initialize library" + std::to_string(result));
        return false;
    }

    // Create handle
    MP3::mpg = mpg123_new(nullptr, &result);
    if (MP3::mpg == nullptr) {
        Log::writeError("[MP3] Failed to create instance: " + std::to_string(result));
        return false;
    }

    // Enable support for custom file object
    result = mpg123_replace_reader_handle(MP3::mpg, NX::File::readFile, NX::File::seekFile, nullptr);
    if (result != MPG123_OK) {
        Log::writeError("[MP3] Unable to enable custom file object support: " + std::to_string(result));
        return false;
    }

    // Enable gapless decoding
    result = mpg123_param(MP3::mpg, MPG123_FLAGS, MPG123_QUIET | MPG123_GAPLESS, 0.0f);
    if (result != MPG123_OK) {
        MP3::logErrorMsg();
        Log::writeWarning("[MP3] Unable to set quiet + gapless flags: " + std::to_string(result));
    }

    // Use fuzzy seeking by default
    MP3::setAccurateSeek(false);

    Log::writeSuccess("[MP3] Initialized successfully");
    return true;
}


void MP3::freeLib() {
    // Delete handle
    if (MP3::mpg != nullptr) {
        Log::writeSuccess("[MP3] Library tidied up!");
        mpg123_delete(MP3::mpg);
        MP3::mpg = nullptr;
    }
}

bool MP3::setAccurateSeek(bool b) {
    // Sanity check
    if (MP3::mpg == nullptr) {
        return false;
    }

    int result = mpg123_param(MP3::mpg, (b ? MPG123_REMOVE_FLAGS : MPG123_ADD_FLAGS), MPG123_FUZZY, 0.0f);
    if (result != MPG123_OK) {
        MP3::logErrorMsg();
        Log::writeWarning("[MP3] Unable to toggle fuzzy seeking");
        return false;
    }

    return true;
}

bool MP3::setEqualizer(const std::array<float, 32> & eq) {
    // Sanity check
    if (MP3::mpg == nullptr) {
        return false;
    }

    // Iterate over array and set each eq band
    int result;
    for (size_t i = 0; i < eq.size(); i++) {
        result = mpg123_eq(MP3::mpg, MPG123_LR, i, eq[i]);
        if (result != MPG123_OK) {
            MP3::logErrorMsg();
            Log::writeError("[MP3] Failed to adjust equalizer band " + std::to_string(i));
            return false;
        }
    }

    return true;
}