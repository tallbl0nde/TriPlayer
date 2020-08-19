#include <algorithm>
#include <chrono>
#include <filesystem>
#include <future>
#include "LibraryScanner.hpp"
#include "utils/MP3.hpp"

#include "Log.hpp"

// Number of threads to use for scanning audio files
#define SCAN_THREADS 2

// Comparator for FilePairs returning true if the lhs is before the rhs
// (this only comapres the path as we don't care about the modified time)
bool LibraryScanner::FilePairComparator(const FilePair & lhs, const FilePair & rhs) {
    return lhs.path < rhs.path;
}

LibraryScanner::LibraryScanner(const SyncDatabase & db, const std::string & path) : database(db), searchPath(path) {

}

LibraryScanner::Status LibraryScanner::parseFileAdd(const FilePair & file) {
    // Read tags and data from file (thread-safe)
    Metadata::Song meta = Utils::MP3::getInfoFromID3(file.path);
    if (meta.ID == -3) {
        return Status::ErrUnknown;
    }
    meta.path = file.path;
    meta.modified = file.modifiedTime;

    // Append to metadata vector
    std::scoped_lock<std::mutex> mtx(this->addMutex);
    this->addMeta.push_back(meta);
    return Status::Ok;
}

LibraryScanner::Status LibraryScanner::parseFileUpdate(const FilePair & file) {
    // Read new tags and data from file (thread-safe)
    Metadata::Song newMeta = Utils::MP3::getInfoFromID3(file.path);
    if (newMeta.ID == -3) {
        return Status::ErrUnknown;
    }

    // Read old data from database (also thread-safe due to wrapper) and merge
    std::string tmp = file.path;
    SongID id = this->database->getSongIDForPath(tmp);
    Metadata::Song meta = this->database->getSongMetadataForID(id);
    if (meta.ID < 0) {
        return Status::ErrDatabase;
    }
    meta.title = newMeta.title;
    meta.artist = newMeta.artist;
    meta.album = newMeta.album;
    meta.duration = newMeta.duration;
    meta.trackNumber = newMeta.trackNumber;
    meta.discNumber = newMeta.discNumber;
    meta.modified = file.modifiedTime;

    // Append to metadata vector
    std::scoped_lock<std::mutex> mtx(this->updateMutex);
    this->updateMeta.push_back(meta);
    return Status::Ok;
}

LibraryScanner::Status LibraryScanner::processFiles() {
    // First get all paths within folder along with modified timestamp
    std::vector<FilePair> files;
    for (auto & entry: std::filesystem::recursive_directory_iterator(this->searchPath)) {
        if (entry.path().extension() == ".mp3") {
            // Why is this conversion so hard?
            auto time = entry.last_write_time();
            auto clock = std::chrono::file_clock::to_sys(time);
            unsigned int timestamp = (unsigned int)std::chrono::system_clock::to_time_t(clock);

            files.push_back(FilePair{entry.path().string(), timestamp});
        }
    }

    // Sort returned paths
    std::sort(files.begin(), files.end(), FilePairComparator);

    // Next get all paths and modified times from database
    // (the database returns paths in sorted order)
    bool dbOK;
    std::vector<FilePair> dbFiles;
    std::vector< std::pair<std::string, unsigned int> > tmp = this->database->getAllSongFileInfo(dbOK);
    if (!dbOK) {
        return Status::ErrDatabase;
    }
    for (size_t i = 0; i < tmp.size(); i++) {
        dbFiles.push_back(FilePair{tmp[i].first, tmp[i].second});
    }

    // Use a thread to work out what files to add
    std::future<void> addThread = std::async(std::launch::async, [this, &files, &dbFiles]() {
        // Check if each file has an entry in the database
        // If not, it needs to be added
        for (size_t i = 0; i < files.size(); i++) {
            bool inDB = std::binary_search(dbFiles.begin(), dbFiles.end(), files[i], FilePairComparator);
            if (!inDB) {
                this->addFiles.push_back(files[i]);
            }
        }
    });

    // Use another thread to work out what files need updating
    std::future<void> updateThread = std::async(std::launch::async, [this, &files, &dbFiles]() {
        // Check if each file is in the database
        // If it is and the DB's modified time is smaller, it needs to be updated
        for (size_t i = 0; i < files.size(); i++) {
            std::vector<FilePair>::iterator it = std::lower_bound(dbFiles.begin(), dbFiles.end(), files[i], FilePairComparator);
            if (it != dbFiles.end() && (*it).path == files[i].path) {
                if ((*it).modifiedTime < files[i].modifiedTime) {
                    this->updateFiles.push_back(files[i]);
                }
            }
        }
    });

    // This thread is responsible for determining which files to remove
    for (size_t i = 0; i < dbFiles.size(); i++) {
        bool onSD = std::binary_search(files.begin(), files.end(), dbFiles[i], FilePairComparator);
        if (!onSD) {
            this->removeFiles.push_back(dbFiles[i]);
        }
    }

    // Wait for threads to finish
    addThread.get();
    updateThread.get();

    // Return appropriate status
    if (this->addFiles.empty() && this->updateFiles.empty()) {
        return (this->removeFiles.empty() ? Status::Done : Status::DoneRemove);
    }
    return Status::Ok;
}

LibraryScanner::Status LibraryScanner::processMetadata(std::atomic<int> & currentFile, std::atomic<int> & totalFiles, std::atomic<size_t> & estRemaining) {
    // Set initial status values
    estRemaining = 0;
    currentFile = 1;
    totalFiles = this->addFiles.size() + this->updateFiles.size();

    // We're going to use multiple threads to hopefully speed up the parsing
    std::vector< std::future<Status> > threads;

    // Parse files that need to be added/updated
    std::vector<FilePair> dummy;
    Status status = Status::Ok;
    for (size_t v = 0; v < 2; v++) {
        // Pick vector based on 'v' (used to avoid repeating code)
        std::vector<FilePair> & vec = dummy;
        switch (v) {
            case 0:
                vec = this->addFiles;
                break;

            case 1:
                vec = this->updateFiles;
                break;

            default:
                return Status::ErrUnknown;
        }

        // Now actually iterate over each entry
        size_t nextIdx = 0;
        do {
            // First check if threads are done
            size_t i = 0;
            while (i < threads.size()) {
                if (threads[i].wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    status = (status == Status::Ok ? threads[i].get() : status);
                    threads.erase(threads.begin() + i);
                    currentFile++;
                } else {
                    i++;
                }
            }

            // Then enqueue next file(s)
            if (status == Status::Ok) {
                while (threads.size() < SCAN_THREADS) {
                    if (nextIdx >= vec.size()) {
                        break;
                    }

                    FilePair f = vec[nextIdx];
                    threads.emplace_back(std::async(std::launch::async, [this, f, v]() -> Status {
                        return (v == 0 ? this->parseFileAdd(f) : this->parseFileUpdate(f));
                    }));
                    nextIdx++;
                }

                // Wait very briefly before checking again
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

        } while (nextIdx < vec.size() && status == Status::Ok);

        // Return if an error occurred
        if (status != Status::Ok) {
            while (!threads.empty()) {
                threads[0].get();
                threads.erase(threads.begin());
            }
            return status;
        }
    }

    // We get here once all are completed and no error occurred
    return Status::Ok;
}

LibraryScanner::Status LibraryScanner::updateDatabase() {
    // Add songs first
    for (size_t i = 0; i < this->addMeta.size(); i++) {
        bool ok = this->database->addSong(this->addMeta[i]);
        if (!ok) {
            return Status::ErrDatabase;
        }
    }

    // Then update songs
    for (size_t i = 0; i < this->updateMeta.size(); i++) {
        bool ok = this->database->updateSong(this->updateMeta[i]);
        if (!ok) {
            return Status::ErrDatabase;
        }
    }

    // And finally remove songs
    bool ok = true;
    for (size_t i = 0; i < this->removeFiles.size(); i++) {
        std::string tmp = this->removeFiles[i].path;
        SongID id = this->database->getSongIDForPath(tmp);
        (id >= 0 ? ok = this->database->removeSong(id) : ok = false);
        if (!ok) {
            return Status::ErrDatabase;
        }
    }

    return Status::Ok;
}