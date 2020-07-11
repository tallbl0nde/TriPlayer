#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mpg123.h>
#include "Log.hpp"
#include "utils/MP3.hpp"

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

    // Calculates duration of MP3 file
    // Returns number of seconds (0 if error occurred)
    static unsigned int parseDuration(std::ifstream &file) {
        // Read file into RAM
        file.seekg(0, file.end);
        size_t size = file.tellg();
        file.seekg(0, file.beg);
        unsigned char * buf;
        try {
            buf = new unsigned char[size];
        } catch (const std::bad_alloc& e) {
            // For now just skip files that are too large
            return 0;
        }
        file.read((char *) buf, size);
        size_t pos = 0;

        // If an ID3 tag is present skip it
        if (buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3') {
            unsigned int tagSize = 0;
            tagSize = ((buf[6] & 127) << 21) | ((buf[7] & 127) << 14) | ((buf[8] & 127) << 7) | ((buf[9] & 127));
            pos += 10 + tagSize;
        }

        // Loop over each frame and calculate stuff
        double seconds = 0;
        while (pos < size) {
            // Read frame header
            if (!(buf[pos] == 0xFF && (buf[pos + 1] & 0b11100000) == 0b11100000)) {
                break;
            }

            // Get mpeg version (layer is always 3!)
            MPEGVer ver = MPEGVer::One;
            switch (buf[pos + 1] & 0b00011000) {
                case 0b00011000:
                    ver = MPEGVer::One;
                    break;

                case 0b00010000:
                    ver = MPEGVer::Two;
                    break;

                case 0b00000000:
                    ver = MPEGVer::TwoFive;
                    break;
            }

            // Check for padding (which is 1 byte)
            bool padding = ((buf[pos + 2] & 0b10) == 0b10);

            // Get bitrate and sample rate
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

            int samplerate = 1;
            switch (buf[pos + 2] & 0b1100) {
                case 0b0000:
                    samplerate = 44100;
                    break;

                case 0b0100:
                    samplerate = 48000;
                    break;

                case 0b1000:
                    samplerate = 32000;
                    break;
            }
            if (ver == MPEGVer::Two) {
                samplerate /= 2;
            } else if (ver == MPEGVer::TwoFive) {
                samplerate /= 4;
            }

            // Calculate size and jump to end (next frame header)
            unsigned int size = ((144 * bitrate) / samplerate) + (padding ? 1 : 0) - 4;
            pos += 4 + size;

            // Duration in seconds is (samples per frame / sample rate)
            seconds += (1152 / (long double)samplerate);
        }

        // Delete RAM copy
        delete[] buf;

        return (unsigned int)std::round(seconds);
    }

    // Parses ID3v1 tags and fills SongInfo
    static void parseID3V1(mpg123_id3v1 * v1, Metadata::Song & info) {
        info.title = arrayToString(v1->title, sizeof(v1->title));
        info.artist = arrayToString(v1->artist, sizeof(v1->artist));
        info.album = arrayToString(v1->album, sizeof(v1->album));
    }

    // Parses ID3v2 tags and fills SongInfo
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
    Metadata::AlbumArt getArtFromID3(std::string path) {
        Metadata::AlbumArt m;
        m.data = nullptr;
        m.size = 0;

        // Use mpg123 to find images
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
                                        // Copy image into struct
                                        m.data = new unsigned char[pic->size];
                                        std::memcpy(m.data, pic->data, pic->size);
                                        m.size = pic->size;
                                        break;
                                    }
                                }
                            }

                            // Log if none found
                            if (m.data == nullptr) {
                                Log::writeWarning("[MP3] No suitable art found in: " + path);
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

        return m;
    }

    // Checks for tag type and calls appropriate function
    Metadata::Song getInfoFromID3(std::string path) {
        // Default info to return
        Metadata::Song m;
        m.ID = -3;
        m.title = std::filesystem::path(path).stem();      // Title defaults to file name
        m.artist = "Unknown Artist";                       // Artist defaults to unknown
        m.album = "Unknown Album";                         // Same for album

        // Use mpg123 to read ID3 tags
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
                }
                // Close file
                mpg123_close(mpg);

            } else {
                Log::writeError("[MP3] Unable to open file: " + path);
            }
        }

        // Calculate duration
        std::ifstream file;
        file.open(path, std::ios::binary | std::ios::in);
        file.seekg(0, file.beg);
        m.duration = parseDuration(file);

        return m;
    }
};