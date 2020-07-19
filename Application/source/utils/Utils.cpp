#include <algorithm>
#include <cmath>
#include <filesystem>
#include <sys/stat.h>
#include "utils/MP3.hpp"
#include "utils/Utils.hpp"

namespace Utils {
    time_t getModifiedTimestamp(std::string path) {
        struct stat st;
        if (stat(path.c_str(), &st) == 0) {
            return st.st_mtime;
        }
        return 0;
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

    void processFileChanges(Database * db, Sysmodule * sys, std::atomic<int> & aFile, std::atomic<ProcessStage> & aStage, std::atomic<int> & aTotal, std::atomic<bool> & stop) {
        // Get list of file paths
        aStage = ProcessStage::Search;
        std::vector<std::string> paths = getFilesWithExt("/music", ".mp3");

        // Have an 'adjacent' vector which stores modified time
        std::vector<time_t> diskMTime;
        for (size_t i = 0; i < paths.size(); i++) {
            diskMTime.push_back(getModifiedTimestamp(paths[i]));
        }

        // Iterate over vector(s) and get data to insert/edit
        db->close();
        db->openReadOnly();
        std::vector<size_t> addPos;
        std::vector<size_t> editPos;
        for (size_t i = 0; i < paths.size(); i++) {
            unsigned int dataMTime = db->getModifiedTimeForPath(paths[i]);

            // DB time will be 0 if no entry exists!
            if (dataMTime == 0) {
                addPos.push_back(i);

            // DB time will be behind if file changed
            } else if (diskMTime[i] > dataMTime) {
                editPos.push_back(i);
            }

            // Stop if requested
            if (stop) {
                break;
            }
        }

        // Lock Database for writing if necessary
        bool hasLock = false;
        if (!stop) {
            if (addPos.size() + editPos.size() != 0) {
                hasLock = true;
                db->close();
                sys->waitReset();
                sys->waitRequestDBLock();
                db->openReadWrite();

                // Actually insert/update now (this is messy just so the user can get the status...)
                aTotal = addPos.size() + editPos.size();
                aStage = ProcessStage::Parse;
                for (size_t i = 0; i < addPos.size(); i++) {
                    if (stop) {
                        break;
                    }

                    aFile++;
                    Metadata::Song info = Utils::MP3::getInfoFromID3(paths[addPos[i]]);
                    info.path = paths[addPos[i]];
                    info.modified = diskMTime[addPos[i]];
                    db->addSong(info);
                }
                for (size_t i = 0; i < editPos.size(); i++) {
                    if (stop) {
                        break;
                    }

                    aFile++;

                    // Overwrite existing metadata
                    SongID id = db->getSongIDForPath(paths[editPos[i]]);
                    Metadata::Song info = db->getSongMetadataForID(id);
                    Metadata::Song tmp = Utils::MP3::getInfoFromID3(paths[editPos[i]]);
                    info.title = tmp.title;
                    info.artist = tmp.artist;
                    info.album = tmp.album;
                    info.duration = tmp.duration;
                    info.modified = diskMTime[editPos[i]];
                    db->updateSong(info);
                }
            }
        }

        // Remove any deleted songs (+ lock if not already done so)
        if (!stop) {
            std::vector<std::string> dbPaths = db->getAllSongPaths();
            for (size_t i = 0; i < dbPaths.size(); i++) {
                if (stop) {
                    break;
                }

                // If DB's path is not present remove it!
                if (std::find(paths.begin(), paths.end(), dbPaths[i]) == paths.end()) {
                    // Lock if not done in last step
                    if (!hasLock) {
                        aStage = ProcessStage::Update;
                        hasLock = true;
                        db->close();
                        sys->waitReset();
                        sys->waitRequestDBLock();
                        db->openReadWrite();
                    }
                    db->removeSong(db->getSongIDForPath(dbPaths[i]));
                }
            }
        }

        // Update search tables if needed
        if (db->needsSearchUpdate()) {
            if (!hasLock) {
                aStage = ProcessStage::Update;
                hasLock = true;
                db->close();
                sys->waitReset();
                sys->waitRequestDBLock();
                db->openReadWrite();
            }
            db->prepareSearch();
        }

        // Cleanup database (TBD)
        if (hasLock) {
            db->close();
            sys->sendReleaseDBLock();
            db->openReadOnly();
        }

        aStage = ProcessStage::Done;
    }

    // From: https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c/12468109#12468109
    static bool seedSet = false;
    std::string randomString(size_t length) {
        // Set seed if not already set
        if (!seedSet) {
            srand(time(NULL));
            seedSet = true;
        }

        auto randchar = []() -> char {
            const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[rand() % max_index];
        };
        std::string str(length, 0);
        std::generate_n(str.begin(), length, randchar);
        return str;
    }

    float roundToDecimalPlace(float val, unsigned int p) {
        for (unsigned int i = 0; i < p; i++) {
            val *= 10.0;
        }
        val = std::round(val);
        for (unsigned int i = 0; i < p; i++) {
            val /= 10.0;
        }
        return val;
    }

    std::string secondsToHMS(unsigned int sec) {
        std::string str = "";
        // Hours
        int h = sec/3600;
        if (h > 0) {
            str += std::to_string(h) + ":";
        }

        // Minutes
        int m = ((sec/60)%60);
        if (str.length() > 0 && m < 10) {
            str += "0";
        }
        str += std::to_string(m);

        // Seconds
        str += ":";
        int s = sec%60;
        if (s < 10) {
            str += "0";
        }
        str += std::to_string(s);

        return str;
    }

    std::string secondsToHoursMins(unsigned int sec) {
        std::string str = "";

        // Hours
        int h = sec/3600;
        if (h != 0) {
            str += std::to_string(h);
        }
        if (h == 1) {
            str += " hour";
        } else if (h != 0) {
            str += " hours";
        }

        // Minutes
        int m = ((sec/60)%60);
        if (h > 0) {
            str += ", ";
        }
        str += std::to_string(m);
        if (m == 1) {
            str += " minute";
        } else {
            str += " minutes";
        }

        return str;
    }

    std::vector<std::string> splitIntoWords(const std::string & str) {
        std::vector<std::string> words;

        // Iterate over each word
        std::string word = "";
        size_t pos = 0;
        while (pos < str.length()) {
            // Append chars to word until a space is reached
            if (str[pos] != ' ') {
                word.append(1, str[pos]);
            } else {
                // Don't add empty words (i.e. due to repeated spaces)
                if (word.length() > 0) {
                    words.push_back(word);
                    word = "";
                }
            }
            pos++;
        }

        // Check we haven't missed the last word
        if (word.length() > 0) {
            words.push_back(word);
        }

        return words;
    }

    std::string truncateToDecimalPlace(std::string str, unsigned int p) {
        size_t dec = str.find(".");
        if (dec == std::string::npos || p >= str.length() - dec) {
            return str;
        }

        // Cut off decimal place if zero
        if (p == 0) {
            dec--;
        }

        return str.substr(0, dec + 1 + p);
    }
};