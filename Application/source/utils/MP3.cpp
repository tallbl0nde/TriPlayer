#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mpg123.h>
#include <mutex>
#include "Log.hpp"
#include "utils/MP3.hpp"

// Size of read buffer (used for calculating duration) - 20MB
// This should be enough for most songs, but also large enough to not cause too many
// reads for very large files
#define READ_BUFFER_SIZE 20 * 1024 * 1024

// Bitrates matching value of bitrate bits
static const int bitVer1[16] = {0, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000, 0};
static const int bitVer2[16] = {0, 32000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000, 176000, 192000, 224000, 256000, 0};

// MPEG Version
enum class MPEGVer {
    One,        // 1
    Two,        // 2
    TwoFive     // 2.5
};

// mpg123 instance
static mpg123_handle * mpg = nullptr;

// Mutex protecting above handle
static std::mutex mutex;

namespace Utils::MP3 {
    // Small helper function which returns a string from a (possibly not) null terminated string
    static std::string arrayToString(char * data, size_t size) {
        // Ensure it's null terminated
        char arr[size + 1];
        std::memcpy(arr, data, size);
        arr[size] = '\0';

        return std::string(arr);
    }

    static std::string mpgStrToString(mpg123_string * str) {
        if (str == nullptr) {
            return "";
        }

        // mpg123_string can contain multiple strings - we only want the first one
        size_t len = std::strlen(str->p);
        return std::string(str->p, len);
    }

    // MISSING ~4 HOURS OF DURATION

    // Calculates duration of MP3 file and returns in seconds (0 if error occurred)
    // If you're reading this and you're interested how I came up with this,
    // see this site: http://www.mp3-tech.org/programmer/frame_header.html
    static unsigned int parseDuration(std::ifstream &file) {
        double seconds = 0;

        // Read buffer
        unsigned char * buf = new unsigned char[READ_BUFFER_SIZE];
        size_t size = 0;

        // Position to begin reading from in next buffer
        size_t beginAt = 0;

        bool atStart = true;
        while (file) {
            // Read chunks of the file until we reach the end
            file.read((char *) buf + size, READ_BUFFER_SIZE - size);
            size += file.gcount();
            if (!size) {
                break;
            }

            // Byte we are 'at' in this buffer
            size_t pos = beginAt;

            // Check for the ID3 tag if at the start
            if (atStart) {
                // Stop if we didn't read 10 bytes
                if (size < 10) {
                    seconds = 0;
                    break;
                }

                // If we have an ID3 tag skip it
                if (buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3') {
                    unsigned int tagSize = 0;
                    tagSize = ((buf[6] & 127) << 21) | ((buf[7] & 127) << 14) | ((buf[8] & 127) << 7) | ((buf[9] & 127));
                    pos += 10 + tagSize;
                }
            }

            // Loop over MPEG frames in the buffer until we run out
            bool loop = true;
            while (pos < size && size - pos >= 3) {
                // Check for the header and stop once we encounter something that isn't right
                if (!(buf[pos] == 0xFF && (buf[pos + 1] & 0b11100000) == 0b11100000)) {
                    loop = false;
                    break;
                }

                // Get mpeg version (layer is always 3!)
                MPEGVer ver = MPEGVer::One;
                switch ((buf[pos + 1] & 0b00011000) >> 3) {
                    case 0b11:
                        ver = MPEGVer::One;
                        break;

                    case 0b10:
                        ver = MPEGVer::Two;
                        break;

                    case 0b00:
                        ver = MPEGVer::TwoFive;
                        break;
                }

                // Check for CRC bytes
                bool hasCRC = ((buf[pos + 1] & 0b1) == 0b0);

                // Get bitrate
                int bitrate;
                switch (ver) {
                    case MPEGVer::One:
                        bitrate = bitVer1[(buf[pos + 2] & 0b11110000) >> 4];
                        break;

                    case MPEGVer::Two:
                    case MPEGVer::TwoFive:
                        bitrate = bitVer2[(buf[pos + 2] & 0b11110000) >> 4];
                        break;
                }

                // Get sample rate
                int samplerate = 1;
                switch ((buf[pos + 2] & 0b00001100) >> 2) {
                    case 0b00:
                        samplerate = 44100;
                        break;

                    case 0b01:
                        samplerate = 48000;
                        break;

                    case 0b10:
                        samplerate = 32000;
                        break;
                }
                if (ver == MPEGVer::Two) {
                    samplerate /= 2;
                } else if (ver == MPEGVer::TwoFive) {
                    samplerate /= 4;
                }

                // Check for padding
                bool hasPadding = (((buf[pos + 2] & 0b10) >> 1) == 0b1);

                // Calculate size and jump to next frame header
                unsigned int frameSize = ((144 * bitrate) / samplerate) + (hasCRC ? 2 : 0) + (hasPadding ? 1 : 0) - 4;
                pos += (4 + frameSize);

                // Duration in seconds is (samples per frame / sample rate)
                seconds += (1152 / (long double)samplerate);
            }

            // Break immediately on error
            if (!loop) {
                break;
            }

            // Move remaining bytes to the start of the buffer
            if (pos < size) {
                size -= pos;
                for (size_t i = 0; i < size; i++) {
                    buf[i] = buf[pos + i];
                }
                beginAt = 0;

            // Otherwise indicate what byte in the next buffer will start the next header
            } else {
                beginAt = (pos - size);
                size = 0;
            }
            atStart = false;
        }

        delete[] buf;
        return (unsigned int)std::round(seconds);
    }

    // Parses ID3v1 tags and fills Metadata struct
    static void parseID3V1(mpg123_id3v1 * v1, Metadata::Song & info) {
        info.title = arrayToString(v1->title, sizeof(v1->title));
        info.artist = arrayToString(v1->artist, sizeof(v1->artist));
        info.album = arrayToString(v1->album, sizeof(v1->album));
    }

    // Parses ID3v2 tags and fills Metadata struct
    static void parseID3V2(mpg123_id3v2 * v2, Metadata::Song & info) {
        std::string tmp;
        if (v2->title != nullptr) {
            tmp = mpgStrToString(v2->title);
            if (tmp.length() > 0) {
                info.title = tmp;
            }
        }
        if (v2->artist != nullptr) {
            tmp = mpgStrToString(v2->artist);
            if (tmp.length() > 0) {
                info.artist = tmp;
            }
        }
        if (v2->album != nullptr) {
            tmp = mpgStrToString(v2->album);
            if (tmp.length() > 0) {
                info.album = tmp;
            }
        }

        // To find other metadata we need to iterate over each field
        char done = 0x0;
        for (size_t i = 0; i < v2->texts; i++) {
            mpg123_text t = v2->text[i];

            // Track number
            if (t.id[0] == 'T' && t.id[1] == 'R' && t.id[2] == 'C' && t.id[3] == 'K') {
                done |= 0x01;
                errno = 0;
                int tmp = std::atoi(t.text.p);
                if (errno == 0) {
                    info.trackNumber = tmp;
                }
                continue;
            }

            // Disc number
            if (t.id[0] == 'T' && t.id[1] == 'P' && t.id[2] == 'O' && t.id[3] == 'S') {
                done |= 0x10;
                errno = 0;
                int tmp = std::atoi(t.text.p);
                if (errno == 0) {
                    info.discNumber = tmp;
                }
                continue;
            }

            // Stop when all are found
            if (done == 0x11) {
                break;
            }
        }
    }

    bool init() {
        // Prepare library
        int err = mpg123_init();
        if (err != MPG123_OK) {
            Log::writeError("[MP3] Failed to init mpg123");
            return false;
        }

        // Create instance
        mpg = mpg123_new(nullptr, &err);
        if (err != MPG123_OK) {
            Log::writeError("[MP3] Failed to create a mpg123 instance");
            mpg = nullptr;
            return false;
        }

        // Store pictures
        err = mpg123_param(mpg, MPG123_ADD_FLAGS, MPG123_PICTURE, 0.0);
        if (err != MPG123_OK) {
            Log::writeError("[MP3] Failed to set MPG123_PICTURE flag");
            return false;
        }

        Log::writeSuccess("[MP3] mpg123 initialized sucessfully!");
        return true;
    }

    void exit() {
        if (mpg != nullptr) {
            mpg123_delete(mpg);
        }
        mpg = nullptr;
    }

    // Searches and returns an appropriate image
    std::vector<unsigned char> getArtFromID3(std::string path) {
        std::vector<unsigned char> v;

        // Use mpg123 to find images
        std::scoped_lock<std::mutex> mtx(mutex);
        if (mpg != nullptr) {
            int err = mpg123_open(mpg, path.c_str());
            if (err == MPG123_OK) {
                // Check if there are ID3 tags
                mpg123_seek(mpg, 0, SEEK_SET);
                if (mpg123_meta_check(mpg) & MPG123_ID3) {
                    // Structures storing metadata (v1 not used)
                    mpg123_id3v1 * v1;
                    mpg123_id3v2 * v2;

                    // Parse metadata
                    err = mpg123_id3(mpg, &v1, &v2);
                    if (err == MPG123_OK) {
                        // Check for ID3v2 tags
                        if (v2 != nullptr) {
                            // Iterate over all images to find a fitting image
                            for (size_t i = 0; i < v2->pictures; i++) {
                                mpg123_picture * pic = &v2->picture[i];
                                std::string mType = mpgStrToString(&(pic->mime_type));

                                // Need matching type and mime type
                                if (pic->type == mpg123_id3_pic_other || pic->type == mpg123_id3_pic_front_cover) {
                                    if (mType == "image/jpg" || mType == "image/jpeg" || mType == "image/png") {
                                        // Copy image into vector
                                        for (size_t i = 0; i < pic->size; i++) {
                                            v.push_back(*(pic->data + i));
                                        }
                                        break;
                                    }
                                }
                            }

                            // Log if none found
                            if (v.empty()) {
                                Log::writeInfo("[MP3] No suitable art found in: " + path);
                            }

                        } else {
                            Log::writeWarning("[MP3] No ID3v2 tags were found in: " + path);
                        }

                    } else {
                        Log::writeError("[MP3] Failed to parse metadata for: " + path);
                    }
                } else {
                    Log::writeError("[MP3] Failed to parse metadata for: " + path);
                }
                // Free memory used by metadata
                mpg123_meta_free(mpg);
            }
            // Close file
            mpg123_close(mpg);
        } else {
            Log::writeError("[MP3] Unable to open file: " + path);
        }

        return v;
    }

    // Checks for tag type and calls appropriate function
    Metadata::Song getInfoFromID3(std::string path) {
        // Default info to return
        Metadata::Song m;
        m.ID = -3;
        m.title = std::filesystem::path(path).stem();      // Title defaults to file name
        m.artist = "Unknown Artist";                       // Artist defaults to unknown
        m.album = "Unknown Album";                         // Same for album
        m.trackNumber = 0;                                 // Initially 0 to indicate not set
        m.discNumber = 0;                                  // Initially 0 to indicate not set

        // Use mpg123 to read ID3 tags
        std::unique_lock<std::mutex> mtx(mutex);
        if (mpg != nullptr) {
            int err = mpg123_open(mpg, path.c_str());
            if (err == MPG123_OK) {
                // Check if there are ID3 tags
                mpg123_seek(mpg, 0, SEEK_SET);
                if (mpg123_meta_check(mpg) & MPG123_ID3) {
                    // Structures storing metadata
                    mpg123_id3v1 * v1;
                    mpg123_id3v2 * v2;

                    // Parse metadata
                    err = mpg123_id3(mpg, &v1, &v2);
                    if (err == MPG123_OK) {
                        // Check for ID3v2 tags first
                        if (v2 != nullptr) {
                            m.ID = -1;
                            parseID3V2(v2, m);

                        } else if (v1 != nullptr) {
                            m.ID = -1;
                            parseID3V1(v1, m);

                        } else {
                            // No tags found!
                            m.ID = -2;
                            Log::writeWarning("[MP3] No tags were found in: " + path);
                        }

                    } else {
                        Log::writeError("[MP3] Failed to parse metadata for: " + path);
                    }
                    // Free memory used by metadata
                    mpg123_meta_free(mpg);

                } else {
                    m.ID = -2;
                    Log::writeWarning("[MP3] No ID3 metadata present in: " + path);
                }

                // Close file
                mpg123_close(mpg);

            } else {
                Log::writeError("[MP3] Unable to open file: " + path);
            }
        }
        mtx.unlock();

        // Calculate duration
        std::ifstream file;
        file.open(path, std::ios::binary | std::ios::in);
        file.seekg(0, file.beg);
        m.duration = parseDuration(file);

        return m;
    }
};