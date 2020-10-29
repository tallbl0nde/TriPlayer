#include <algorithm>
#include <filesystem>
#include <future>
#include "LibraryScanner.hpp"
#include "Log.hpp"
#include "Paths.hpp"
#include "utils/Fs.hpp"
#include "utils/Image.hpp"
#include "utils/MP3.hpp"
#include "utils/Timer.hpp"
#include "utils/Utils.hpp"

// Number of threads to use for scanning audio files
// For my library 2 threads instead of one sped up scanning by ~5%
#define SCAN_THREADS 2

// Comparator for FilePairs returning true if the lhs is before the rhs
// (this only comapres the path as we don't care about the modified time)
bool LibraryScanner::FilePairComparator(const FilePair & lhs, const FilePair & rhs) {
    return lhs.path < rhs.path;
}

LibraryScanner::LibraryScanner(const SyncDatabase & db, const std::string & path) : database(db), searchPath(path) {

}

std::string LibraryScanner::parseAlbumArt(const std::string & path) {
    // First attempt to extract image from file
    std::vector<unsigned char> image = Utils::MP3::getArtFromID3(path);
    if (image.empty()) {
        return "";
    }

    // If we extracted an image resize it
    bool resized = Utils::Image::resize(image, 400, 400);
    if (!resized) {
        Log::writeError("[SCAN] [ART] Unable to resize image found in: " + path);
        return "";
    }

    // Write the image to disk
    std::string filename;
    do {
        filename = Utils::randomString(10);
    } while (Utils::Fs::fileExists(Path::App::AlbumImageFolder + filename + ".png"));
    filename = Path::App::AlbumImageFolder + filename + ".png";
    bool ok = Utils::Fs::writeFile(filename, image);
    if (!ok) {
        Log::writeError("[SCAN] [ART] Unable to write image to file: " + filename);
        return "";
    }

    return filename;
}

LibraryScanner::Status LibraryScanner::parseFileAdd(const FilePair & file) {
    // Read tags and data from file (thread-safe)
    Metadata::Song meta = Utils::MP3::getInfoFromID3(file.path);
    if (meta.ID == -3) {
        Log::writeError("[SCAN] [ADD] Failed to parse file: " + file.path);
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
        Log::writeError("[SCAN] [UPDATE] Failed to parse file: " + file.path);
        return Status::ErrUnknown;
    }

    // Read old data from database (also thread-safe due to wrapper) and merge
    std::string tmp = file.path;
    SongID id = this->database->getSongIDForPath(tmp);
    Metadata::Song meta = this->database->getSongMetadataForID(id);
    if (meta.ID < 0) {
        Log::writeError("[SCAN] [UPDATE] Failed to get metadata for: " + file.path);
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
    Log::writeInfo("[SCAN] Found " + std::to_string(files.size()) + " files");

    // Sort returned paths
    std::sort(files.begin(), files.end(), FilePairComparator);

    // Next get all paths and modified times from database
    // (the database returns paths in sorted order)
    bool dbOK;
    std::vector<FilePair> dbFiles;
    std::vector< std::pair<std::string, unsigned int> > tmp = this->database->getAllSongFileInfo(dbOK);
    if (!dbOK) {
        Log::writeError("[SCAN] Couldn't read filesystem info from database");
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

    // Log status
    Log::writeInfo("[SCAN] Adding " + std::to_string(this->addFiles.size()) + " files");
    Log::writeInfo("[SCAN] Updating " + std::to_string(this->updateFiles.size()) + " files");
    Log::writeInfo("[SCAN] Removing " + std::to_string(this->removeFiles.size()) + " files");
    Log::writeSuccess("[SCAN] Initial processing completed");

    // Return appropriate status
    if (this->addFiles.empty() && this->updateFiles.empty()) {
        return (this->removeFiles.empty() ? Status::Done : Status::DoneRemove);
    }
    return Status::Ok;
}

LibraryScanner::Status LibraryScanner::processMetadata(std::atomic<size_t> & currentFile, std::atomic<size_t> & totalFiles, std::atomic<size_t> & estRemaining) {
    // Set initial status values
    estRemaining = 0;
    currentFile = 1;
    totalFiles = this->addFiles.size() + this->updateFiles.size();

    // We're going to use multiple threads to hopefully speed up the parsing
    std::vector< std::future<Status> > threads;

    // Timer used to estimate remaining time
    Utils::Timer timer = Utils::Timer();
    timer.start();

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

                    // Increment counter and adjust remaining time
                    estRemaining = (timer.elapsedSeconds() / (double)currentFile) * (totalFiles - currentFile);
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
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

        } while (nextIdx < vec.size() && status == Status::Ok);

        // Return if an error occurred
        if (status != Status::Ok) {
            Log::writeError("[SCAN] Error occurred during metadata scan");
            while (!threads.empty()) {
                threads[0].get();
                threads.erase(threads.begin());
            }
            return status;
        }
    }

    // We get here once all are completed and no error occurred
    Log::writeSuccess("[SCAN] Song metadata processed successfully");
    return Status::Ok;
}

LibraryScanner::Status LibraryScanner::updateDatabase() {
    // Add songs first
    for (size_t i = 0; i < this->addMeta.size(); i++) {
        bool ok = this->database->addSong(this->addMeta[i]);
        if (!ok) {
            Log::writeError("[SCAN] Error adding song: " + this->addMeta[i].path);
            return Status::ErrDatabase;
        }
    }

    // Then update songs
    for (size_t i = 0; i < this->updateMeta.size(); i++) {
        bool ok = this->database->updateSong(this->updateMeta[i]);
        if (!ok) {
            Log::writeError("[SCAN] Error updating song: " + this->updateMeta[i].path);
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
            Log::writeError("[SCAN] Error removing song: " + this->removeFiles[i].path);
            return Status::ErrDatabase;
        }
    }

    Log::writeSuccess("[SCAN] Database successfully updated");
    return Status::Ok;
}

LibraryScanner::Status LibraryScanner::processArt(std::atomic<size_t> & currentFile) {
    // Initialize variables
    currentFile = 0;

    // Map used to mark when an album has an image
    // Album name -> bool
    std::unordered_map<std::string, bool> hasImage;

    // First get all the albums in the database and mark
    std::vector<Metadata::Album> albums = this->database->getAllAlbumMetadata(Database::SortBy::AlbumAsc);
    for (size_t i = 0; i < albums.size(); i++) {
        hasImage[albums[i].name] = (!albums[i].imagePath.empty());
    }

    // Iterate over each song added/updated and search for an image if the album doesn't have one
    std::vector<Metadata::Song> dummy;
    for (size_t v = 0; v < 2; v++) {
        // Pick vector based on 'v' (used to avoid repeating code)
        std::vector<Metadata::Song> & vec = dummy;
        switch (v) {
            case 0:
                vec = this->addMeta;
                break;

            case 1:
                vec = this->updateMeta;
                break;

            default:
                return Status::ErrUnknown;
        }

        for (size_t i = 0; i < vec.size(); i++) {
            Metadata::Song meta = vec[i];

            // Check this song for album art if the album does not yet have any
            if (hasImage[meta.album]) {
                continue;
            }
            std::string path = this->parseAlbumArt(meta.path);

            // If the image was written to the SD Card update database
            if (path.empty()) {
                continue;
            }
            SongID songID = this->database->getSongIDForPath(meta.path);
            AlbumID albumID = this->database->getAlbumIDForSong(songID);
            Status status = (songID >= 0 && albumID >= 0 ? Status::Ok : Status::ErrDatabase);
            if (status == Status::Ok) {
                Metadata::Album album = this->database->getAlbumMetadataForID(albumID);
                status = (album.ID >= 0 ? Status::Ok : Status::ErrDatabase);
                if (status == Status::Ok) {
                    album.imagePath = path;
                    status = (this->database->updateAlbum(album) ? Status::Ok : Status::ErrDatabase);
                }
            }

            // Remove the image file if an error occurred and return
            if (status != Status::Ok) {
                Utils::Fs::deleteFile(path);
                return status;
            }

            // Otherwise mark that the album has an image
            currentFile++;
            hasImage[meta.album] = true;
        }
    }

    return Status::Ok;
}