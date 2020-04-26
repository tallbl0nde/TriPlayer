#include <cmath>
#include <filesystem>
#include <fstream>
#include "MP3.hpp"
#include "Utils.hpp"

// Bitrates matching value of bitrate bits
static const int bitVer1[16] = {0, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000, 0};
static const int bitVer2[16] = {0, 32000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000, 176000, 192000, 224000, 256000, 0};

// MPEG Version
enum class MPEGVer {
    One,
    Two,
    TwoFive
};

namespace Utils::MP3 {
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

    // Parses ID3v1/1.1 tags
    static void parseID3v1(SongInfo &si, std::ifstream &file) {
        si.ID = -2;

        // Read relevant info into buffer
        char buf[90];
        file.seekg(-125, file.end);
        file.read(&buf[0], 90);
        char chars = 30;

        // Title
        while (buf[chars - 1] == '\0') {
            chars--;
            if (chars <= 0) {
                break;
            }
        }
        if (chars != 0) {
            si.title = std::string(&buf[0], chars);
            si.ID = -1;
        }

        // Artist
        chars = 30;
        while (buf[30 + chars - 1] == '\0') {
            chars--;
            if (chars <= 0) {
                break;
            }
        }
        if (chars != 0) {
            si.artist = std::string(&buf[30], chars);
            si.ID = -1;
        }

        // Album
        chars = 30;
        while (buf[60 + chars - 1] == '\0') {
            chars--;
            if (chars <= 0) {
                break;
            }
        }
        if (chars != 0) {
            si.album = std::string(&buf[60], chars);
            si.ID = -1;
        }
    }

    // Parses ID3v2.2.0 tags
    static void parseID3v2_2(SongInfo &si, std::ifstream &file) {
        // Get size
        unsigned char buf[10];
        file.read((char *) &buf[0], 10);
        unsigned int size = 0;
        size = ((buf[6] & 127) << 21) | ((buf[7] & 127) << 14) | ((buf[8] & 127) << 7) | ((buf[9] & 127));

        // Don't bother with synchronised files
        if (buf[5] & 0b10000000) {
            return;
        }
        // Don't bother with compression
        if (buf[5] & 0b01000000) {
            return;
        }

        // Iterate over frames looking for title, artist + album
        char found = 0x0;
        while (file.tellg() < size) {
            // Get frame info
            char ID[3];
            unsigned char sizeBytes[3];
            unsigned int fSize = 0;
            file.read(&ID[0], 3);
            file.read((char *) &sizeBytes[0], 3);
            fSize = (sizeBytes[0] << 16) | (sizeBytes[1] << 8) | sizeBytes[2];

            // Read data
            char * data = new char[fSize];
            file.read(data, fSize);

            if (fSize >= 0) {
                // Check if UTF-16 encoded
                bool isUnicode = ((*(data) == 0) ? false : true);

                // Now actually check frame type etc
                if (ID[0] == 'T' && ID[1] == 'T' && ID[2] == '2') {
                    // Title found
                    found |= 0x100;
                    si.ID = -1;
                    if (isUnicode) {
                        si.title = ::Utils::unicodeToASCII(data + 1, fSize - 1);
                    } else {
                        while (*(data + fSize - 1) == '\0') {
                            fSize--;
                            if (fSize <= 0) {
                                break;
                            }
                        }
                        if (fSize > 0) {
                            si.title = std::string(data + 1, fSize - 1);
                        }
                    }

                } else if (ID[0] == 'T' && ID[1] == 'P' && ID[2] == '1') {
                    // Artist found
                    found |= 0x010;
                    si.ID = -1;
                    if (isUnicode) {
                        si.artist = ::Utils::unicodeToASCII(data + 1, fSize - 1);
                    } else {
                        while (*(data + fSize - 1) == '\0') {
                            fSize--;
                            if (fSize <= 0) {
                                break;
                            }
                        }
                        if (fSize > 0) {
                            si.artist = std::string(data + 1, fSize - 1);
                        }
                    }

                } else if (ID[0] == 'T' && ID[1] == 'A' && ID[2] == 'L') {
                    // Album found
                    found |= 0x001;
                    si.ID = -1;
                    if (isUnicode) {
                        si.album = ::Utils::unicodeToASCII(data + 1, fSize - 1);
                    } else {
                        while (*(data + fSize - 1) == '\0') {
                            fSize--;
                            if (fSize <= 0) {
                                break;
                            }
                        }
                        if (fSize > 0) {
                            si.album = std::string(data + 1, fSize - 1);
                        }
                    }
                }
            }

            delete[] data;

            // Stop looking if all have been found
            if (found == 0x111) {
                break;
            }
        }

        // If there were no errors set the ID accordingly
        if (found == 0x0) {
            si.ID = -2;
        }
    }

    // Parses ID3v2.3.0 tags
    static void parseID3v2_3(SongInfo &si, std::ifstream &file) {
        // Get size
        unsigned char buf[10];
        file.read((char *) &buf[0], 10);
        unsigned int size = 0;
        size = ((buf[6] & 127) << 21) | ((buf[7] & 127) << 14) | ((buf[8] & 127) << 7) | ((buf[9] & 127));

        // Don't bother with synchronised files
        if (buf[5] & 0b10000000) {
            return;
        }

        // Check for extended header and skip if it exists
        if (buf[5] & 0b01000000) {
            file.read((char *) &buf[0], 10);
            unsigned int exSize = 0;
            exSize = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
            file.seekg(exSize + ((buf[4] & 0b10000000) ? 4 : 0), std::ios::cur);
        }

        // Iterate over frames looking for title, artist + album
        char found = 0x0;
        while (file.tellg() < size) {
            // Get frame info
            char ID[4];
            unsigned char sizeBytes[4];
            unsigned int fSize = 0;
            char flags[2];
            file.read(&ID[0], 4);
            file.read((char *) &sizeBytes[0], 4);
            fSize = (sizeBytes[0] << 24) | (sizeBytes[1] << 16) | (sizeBytes[2] << 8) | sizeBytes[3];
            file.read(&flags[0], 2);

            bool skip = false;
            // Skip ahead if compressed
            if (flags[1] & 0b10000000) {
                file.seekg(4, std::ios::cur);
                skip = true;
            }
            // Skip ahead if encrypted
            if (flags[1] & 0b01000000) {
                file.seekg(1, std::ios::cur);
                skip = true;
            }
            // Skip ahead if group flag set
            if (flags[1] & 0b00100000) {
                file.seekg(1, std::ios::cur);
                skip = true;
            }

            // Read data
            char * data = new char[fSize];
            file.read(data, fSize);

            if (fSize >= 0 && !skip) {
                // Check if UTF-16 encoded
                bool isUnicode = ((*(data) == 0) ? false : true);

                // Now actually check frame type etc
                if (ID[0] == 'T' && ID[1] == 'I' && ID[2] == 'T' && ID[3] == '2') {
                    // Title found
                    found |= 0x100;
                    si.ID = -1;
                    if (isUnicode) {
                        si.title = ::Utils::unicodeToASCII(data + 1, fSize - 1);
                    } else {
                        while (*(data + fSize - 1) == '\0') {
                            fSize--;
                            if (fSize <= 0) {
                                break;
                            }
                        }
                        if (fSize > 0) {
                            si.title = std::string(data + 1, fSize - 1);
                        }
                    }

                } else if (ID[0] == 'T' && ID[1] == 'P' && ID[2] == 'E' && ID[3] == '1') {
                    // Artist found
                    found |= 0x010;
                    si.ID = -1;
                    if (isUnicode) {
                        si.artist = ::Utils::unicodeToASCII(data + 1, fSize - 1);
                    } else {
                        while (*(data + fSize - 1) == '\0') {
                            fSize--;
                            if (fSize <= 0) {
                                break;
                            }
                        }
                        if (fSize > 0) {
                            si.artist = std::string(data + 1, fSize - 1);
                        }
                    }

                } else if (ID[0] == 'T' && ID[1] == 'A' && ID[2] == 'L' && ID[3] == 'B') {
                    // Album found
                    found |= 0x001;
                    si.ID = -1;
                    if (isUnicode) {
                        si.album = ::Utils::unicodeToASCII(data + 1, fSize - 1);
                    } else {
                        while (*(data + fSize - 1) == '\0') {
                            fSize--;
                            if (fSize <= 0) {
                                break;
                            }
                        }
                        if (fSize > 0) {
                            si.album = std::string(data + 1, fSize - 1);
                        }
                    }
                }
            }

            delete[] data;

            // Stop looking if all have been found
            if (found == 0x111) {
                break;
            }
        }

        // If there were no errors set the ID accordingly
        if (found == 0x0) {
            si.ID = -2;
        }
    }

    // Parses ID3v2.4.0 tags
    static void parseID3v2_4(SongInfo &si, std::ifstream &file) {
        // Get size
        unsigned char buf[10];
        file.read((char *) &buf[0], 10);
        unsigned int size = 0;
        size = ((buf[6] & 127) << 21) | ((buf[7] & 127) << 14) | ((buf[8] & 127) << 7) | ((buf[9] & 127));

        // Don't bother with synchronised files
        if (buf[5] & 0b10000000) {
            return;
        }

        // Check for extended header and skip if it exists
        if (buf[5] & 0b01000000) {
            file.read((char *) &buf[0], 10);
            unsigned int exSize = 0;
            exSize = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
            file.seekg(exSize + ((buf[4] & 0b10000000) ? 4 : 0), std::ios::cur);
        }

        // Iterate over frames looking for title, artist + album
        char found = 0x0;
        while (file.tellg() < size) {
            // Get frame info
            char ID[4];
            unsigned char sizeBytes[4];
            unsigned int fSize = 0;
            char flags[2];
            file.read(&ID[0], 4);
            file.read((char *) &sizeBytes[0], 4);
            fSize = ((sizeBytes[0] & 127) << 21) | ((sizeBytes[1] & 127) << 14) | ((sizeBytes[2] & 127) << 7) | ((sizeBytes[3] & 127));
            file.read(&flags[0], 2);

            // Skip ahead if group flag set
            bool skip = false;
            if (flags[1] & 0b01000000) {
                file.seekg(4, std::ios::cur);
                skip = true;
            }
            // Skip ahead if compressed
            if (flags[1] & 0b00001000) {
                file.seekg(1, std::ios::cur);
                skip = true;
            }
            // Skip ahead if encrypted
            if (flags[1] & 0b00000100) {
                file.seekg(1, std::ios::cur);
                skip = true;
            }

            // Read data
            char * data = new char[fSize];
            file.read(data, fSize);

            if (fSize >= 0 && !skip) {
                // Check if UTF-16 encoded
                bool isUnicode = ((*(data) == 0 || *(data) == 3) ? false : true);

                // Now actually check frame type etc
                if (ID[0] == 'T' && ID[1] == 'I' && ID[2] == 'T' && ID[3] == '2') {
                    // Title found
                    found |= 0x100;
                    si.ID = -1;
                    if (isUnicode) {
                        si.title = ::Utils::unicodeToASCII(data + 1, fSize - 1);
                    } else {
                        while (*(data + fSize - 1) == '\0') {
                            fSize--;
                            if (fSize <= 0) {
                                break;
                            }
                        }
                        if (fSize > 0) {
                            si.title = std::string(data + 1, fSize - 1);
                        }
                    }

                } else if (ID[0] == 'T' && ID[1] == 'P' && ID[2] == 'E' && ID[3] == '1') {
                    // Artist found
                    found |= 0x010;
                    si.ID = -1;
                    if (isUnicode) {
                        si.artist = ::Utils::unicodeToASCII(data + 1, fSize - 1);
                    } else {
                        while (*(data + fSize - 1) == '\0') {
                            fSize--;
                            if (fSize <= 0) {
                                break;
                            }
                        }
                        if (fSize > 0) {
                            si.artist = std::string(data + 1, fSize - 1);
                        }
                    }

                } else if (ID[0] == 'T' && ID[1] == 'A' && ID[2] == 'L' && ID[3] == 'B') {
                    // Album found
                    found |= 0x001;
                    si.ID = -1;
                    if (isUnicode) {
                        si.album = ::Utils::unicodeToASCII(data + 1, fSize - 1);
                    } else {
                        while (*(data + fSize - 1) == '\0') {
                            fSize--;
                            if (fSize <= 0) {
                                break;
                            }
                        }
                        if (fSize > 0) {
                            si.album = std::string(data + 1, fSize - 1);
                        }
                    }
                }
            }

            // Move ahead 4 bytes if 'data length indicator' exists
            if (flags[1] & 0b00000001) {
                file.seekg(4, std::ios::cur);
            }

            delete[] data;

            // Stop looking if all have been found
            if (found == 0x111) {
                break;
            }
        }

        // If there were no errors set the ID accordingly
        if (found == 0x0) {
            si.ID = -2;
        }
    }

    // Checks for tag type and calls appropriate function
    SongInfo getInfoFromID3(std::string path) {
        // Default info to return
        SongInfo si;
        si.ID = -3;
        si.title = std::filesystem::path(path).stem();      // Title defaults to file name
        si.artist = "Unknown Artist";                       // Artist defaults to unknown
        si.album = "Unknown Album";                         // Same for album

        // Determine tag type
        std::ifstream file;
        file.open(path, std::ios::binary | std::ios::in);
        short tagType = -1;
        if (file) {
            // Check for "ID3" in first 3 bytes (ID3v2.x)
            char buf[4];
            file.read(&buf[0], 4);
            if (buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3') {
                switch (buf[3]) {
                    // ID3v2.2.0
                    case 2:
                        tagType = 2;
                        break;

                    // ID3v2.3.0
                    case 3:
                        tagType = 3;
                        break;

                    // ID3v2.4.0
                    case 4:
                        tagType = 4;
                        break;
                }
            }

            // Check for "TAG" in (last - 128) bytes (ID3v1.x)
            if (tagType == -1) {
                file.seekg(-128, file.end);
                file.read(&buf[0], 3);
                if (buf[0] == 'T' && buf[1] == 'A' && buf[2] == 'G') {
                    tagType = 1;
                }
            }
        }
        file.seekg(0, file.beg);

        // Call right function
        switch (tagType) {
            case 1:
                parseID3v1(si, file);
                break;

            case 2:
                parseID3v2_2(si, file);
                break;

            case 3:
                parseID3v2_3(si, file);
                break;

            case 4:
                parseID3v2_4(si, file);
                break;
        }

        // Calculate duration
        file.seekg(0, file.beg);
        si.duration = parseDuration(file);

        return si;
    }
};