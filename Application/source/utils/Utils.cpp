#include <filesystem>
#include <fstream>
#include <iostream>
#include "Utils.hpp"

namespace Utils {
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

    SongInfo getInfoFromID3(std::string path) {
        SongInfo si;
        si.ID = -3;
        si.title = path;                    // Title defaults to path
        si.artist = "Unknown Artist";       // Artist defaults to unknown
        si.album = "Unknown Album";         // Same for album

        // For now let's assume the file has an ID3v2 tag (ignoring frame flags)
        std::ifstream file;
        file.open(path, std::ios::binary | std::ios::in);
        if (file) {
            // Check for "ID3" in first 3 bytes
            char buf[10];
            unsigned int size = 0;
            file.read(&buf[0], 10);
            if (buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3') {
                // Ignore ID3v2.4.0
                if (buf[3] == 4) {
                    file.close();
                    return si;
                }

                // Now get size
                size = ((buf[6] & 127) << 21) | ((buf[7] & 127) << 14) | ((buf[8] & 127) << 7) | ((buf[9] & 127));

                // Iterate over frames looking for title, artist + album
                char found = 0x0;
                while (file.tellg() < size) {
                    // Get frame info
                    char ID[4];
                    char sizeBytes[4];
                    unsigned int fSize = 0;
                    char flags[2];
                    file.read(&ID[0], 4);
                    file.read(&sizeBytes[0], 4);
                    fSize = (sizeBytes[0] << 24) | (sizeBytes[1] << 16) | (sizeBytes[2] << 8) | sizeBytes[3];
                    file.read(&flags[0], 2);

                    // Skip ahead some more if compressed
                    if (flags[1] & 0b10000000) {
                        file.seekg(4, std::ios::cur);
                    }

                    // Read data
                    char data[fSize];
                    file.read(&data[0], fSize);

                    if (fSize > 0) {
                        // Determine text encoding type
                        // Only handles 0xFFFE unicode
                        bool isUnicode = true;
                        if (data[0] == 0) {
                            isUnicode = false;
                        }

                        // Now actually check frame type etc
                        if (ID[0] == 'T' && ID[1] == 'I' && ID[2] == 'T' && ID[3] == '2') {
                            // Title found
                            found |= 0x100;
                            si.ID = -1;
                            if (isUnicode) {
                                // Grab every second char (this is sooo wrong but eh)
                                unsigned int chars = (fSize - 3)/2;
                                char tmp[chars];
                                for (size_t i = 0; i < chars * 2; i += 2) {
                                    tmp[i/2] = data[i+3];
                                }
                                while (tmp[chars - 1] == '\0') {
                                    chars--;
                                }
                                si.title = std::string(&tmp[0], chars);
                            } else {
                                while (data[fSize - 1] == '\0') {
                                    fSize--;
                                    if (fSize <= 0) {
                                        break;
                                    }
                                }
                                if (fSize > 0) {
                                    si.title = std::string(&data[1], fSize - 1);
                                }
                            }

                        } else if (ID[0] == 'T' && ID[1] == 'P' && ID[2] == 'E' && ID[3] == '1') {
                            // Artist found
                            found |= 0x010;
                            si.ID = -1;
                            if (isUnicode) {

                            } else {
                                while (data[fSize - 1] == '\0') {
                                    fSize--;
                                    if (fSize <= 0) {
                                        break;
                                    }
                                }
                                if (fSize > 0) {
                                    si.artist = std::string(&data[1], fSize - 1);
                                }
                            }

                        } else if (ID[0] == 'T' && ID[1] == 'A' && ID[2] == 'L' && ID[3] == 'B') {
                            // Album found
                            found |= 0x001;
                            si.ID = -1;
                            if (isUnicode) {

                            } else {
                                while (data[fSize - 1] == '\0') {
                                    fSize--;
                                    if (fSize <= 0) {
                                        break;
                                    }
                                }
                                if (fSize > 0) {
                                    si.album = std::string(&data[1], fSize - 1);
                                }
                            }
                        }

                        // Stop looking if all have been found
                        if (found == 0x111) {
                            break;
                        }
                    }
                }

                if (found == 0x0) {
                    si.ID = -2;
                }
            }
            file.close();
        }

        return si;
    }

    void writeStdout(std::string str) {
        #ifndef _NXLINK_
            return;
        #endif

        std::cout << str << std::endl;
    }
};