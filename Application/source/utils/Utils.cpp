#include <filesystem>
#include <fstream>
#include <iostream>
#include "Utils.hpp"

namespace Utils {
    // 'Converts' UTF-16 to ASCII by dropping the other byte
    // Takes pointer to first char (should be BOM), number of bytes to read
    static std::string unicodeToASCII(char * utf, unsigned int len) {
        unsigned int chars = len/2;
        char * tmp = new char[chars];
        for (size_t i = 0; i < len; i += 2) {
            tmp[i/2] = *(utf + i);
        }
        while (tmp[chars - 1] == '\0') {
            chars--;
        }
        std::string str(&tmp[0], chars);
        delete[] tmp;
        return str;
    }

    std::vector<std::string> getFilesWithExt(std::string dir, std::string ext) {
        std::vector<std::string> paths;

        // Recursive search
        for (auto &p: std::filesystem::recursive_directory_iterator(dir)) {
            if (p.path().extension() == ext) {
                paths.push_back(p.path().string());
            }
        }

        return paths;
    }

    // === BEGIN ID3 PARSING FUNCTIONS ===
    // Parses ID3v1/1.1 tags
    static SongInfo parseID3v1(SongInfo si, std::ifstream file) {
        return si;
    }

    // Parses ID3v2.3.0 tags
    static SongInfo parseID3v2_3(SongInfo si, std::ifstream file) {
        // Get size
        unsigned char buf[10];
        file.read((char *) &buf[0], 10);
        unsigned int size = 0;
        size = ((buf[6] & 127) << 21) | ((buf[7] & 127) << 14) | ((buf[8] & 127) << 7) | ((buf[9] & 127));

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
                        si.title = unicodeToASCII(data + 3, fSize - 3);
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
                        si.artist = unicodeToASCII(data + 3, fSize - 3);
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
                        si.album = unicodeToASCII(data + 3, fSize - 3);
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

        return si;
    }

    // Parses ID3v2.4.0 tags
    static SongInfo parseID3v2_4(SongInfo si, std::ifstream file) {
        return si;
    }

    // Checks for tag type and calls appropriate function
    SongInfo getInfoFromID3(std::string path) {
        // Default info to return
        SongInfo si;
        si.ID = -3;
        si.title = std::filesystem::path(path).filename();    // Title defaults to file name
        si.artist = "Unknown Artist";                       // Artist defaults to unknown
        si.album = "Unknown Album";                         // Same for album

        // Determine tag type
        std::ifstream file;
        file.open(path, std::ios::binary | std::ios::in);
        char tagType = -1;
        if (file) {
            // Check for "ID3" in first 3 bytes (ID3v2.x)
            char buf[4];
            file.read(&buf[0], 4);
            if (buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3') {
                switch (buf[3]) {
                    // ID3v2.3.0
                    case 3:
                        tagType = 1;
                        break;

                    // ID3v2.4.0
                    case 4:
                        tagType = 2;
                        break;
                }
            }

            // Check for "TAG" in (last - 128) bytes (ID3v1.x)
            if (tagType == -1) {
                file.seekg(-128, file.end);
                file.read(&buf[0], 3);
                if (buf[0] == 'T' && buf[1] == 'A' && buf[2] == 'G') {
                    tagType = 0;
                }
            }
        }
        file.seekg(0, file.beg);

        // Call right function
        switch (tagType) {
            case 0:
                si = parseID3v1(si, std::move(file));
                break;

            case 1:
                si = parseID3v2_3(si, std::move(file));
                break;

            case 2:
                si = parseID3v2_4(si, std::move(file));
                break;
        }

        return si;
    }
    // === END ID3 PARSING FUNCTIONS ===

    void writeStdout(std::string str) {
        #ifndef _NXLINK_
            return;
        #endif

        std::cout << str << std::endl;
    }
};