#include <algorithm>
#include <cmath>
#include <filesystem>
#include "MP3.hpp"
#include <sys/stat.h>
#include "Utils.hpp"

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

    void processFileChanges(Database * db, Sysmodule * sys, std::atomic<int> & aFile, std::atomic<ProcessStage> & aStage, std::atomic<int> & aTotal) {
        // Get list of file paths
        aStage = ProcessStage::Search;
        std::vector<std::string> paths = getFilesWithExt("/music", ".mp3");

        // Have an 'adjacent' vector which stores modified time
        std::vector<time_t> diskMTime;
        for (size_t i = 0; i < paths.size(); i++) {
            diskMTime.push_back(getModifiedTimestamp(paths[i]));
        }

        // Iterate over vector(s) and get data to insert/edit
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
        }

        // Lock Database for writing if necessary
        bool hasLock = false;
        if (addPos.size() + editPos.size() != 0) {
            hasLock = true;
            sys->waitReset();
            db->lock();

            // Actually insert/update now (this is messy just so the user can get the status...)
            aTotal = addPos.size() + editPos.size();
            aStage = ProcessStage::Parse;
            for (size_t i = 0; i < addPos.size(); i++) {
                aFile++;
                SongInfo info = Utils::MP3::getInfoFromID3(paths[addPos[i]]);
                db->addSong(info, paths[addPos[i]], diskMTime[addPos[i]]);
            }
            for (size_t i = 0; i < editPos.size(); i++) {
                aFile++;
                SongInfo info = Utils::MP3::getInfoFromID3(paths[editPos[i]]);
                SongID id = db->getSongIDForPath(paths[editPos[i]]);
                db->updateSong(id, info, diskMTime[editPos[i]]);
            }
        }

        // Remove any deleted songs (+ lock if not already done so)
        std::vector<std::string> dbPaths = db->getAllSongPaths();
        for (size_t i = 0; i < dbPaths.size(); i++) {
            // If DB's path is not present remove it!
            if (std::find(paths.begin(), paths.end(), dbPaths[i]) == paths.end()) {
                // Lock if not done in last step
                if (!hasLock) {
                    aStage = ProcessStage::Update;
                    hasLock = true;
                    sys->waitReset();
                    db->lock();
                }
                db->removeSong(db->getSongIDForPath(dbPaths[i]));
            }
        }

        // Cleanup database (TBD)
        if (hasLock) {
            db->cleanup();
            db->unlock();
        }

        aStage = ProcessStage::Done;
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