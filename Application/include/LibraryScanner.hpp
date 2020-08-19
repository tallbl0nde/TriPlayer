#ifndef LIBRARYSCANNER_HPP
#define LIBRARYSCANNER_HPP

#include "db/SyncDatabase.hpp"
#include <mutex>
#include <string>
#include "Types.hpp"
#include <vector>

// The LibraryScanner class searches for audio files in the given path and updates
// the database where necessary.
class LibraryScanner {
    public:
        // Statuses returned by class' methods
        enum class Status {
            Ok,                 // No error occurred
            ErrDatabase,        // The database object had an error
            ErrUnknown,         // Something unexpected went wrong
            DoneRemove,         // Returned when there are only songs to remove
            Done                // Returned when no action needs to be taken
        };

    private:
        // File pair containing path and modified time
        struct FilePair {
            std::string path;           // File path
            unsigned int modifiedTime;  // Last modified timestamp
        };
        static bool FilePairComparator(const FilePair &, const FilePair &);

        // Reference to Database object
        const SyncDatabase & database;
        // Path to search
        const std::string searchPath;

        // Vectors of files to add to database
        std::vector<FilePair> addFiles;
        std::vector<Metadata::Song> addMeta;
        std::mutex addMutex;

        // Vectors of files to update within database
        std::vector<FilePair> updateFiles;
        std::vector<Metadata::Song> updateMeta;
        std::mutex updateMutex;

        // Vector of files to remove
        std::vector<FilePair> removeFiles;

        // Functions to actually process files on another thread
        Status parseFileAdd(const FilePair &);
        Status parseFileUpdate(const FilePair &);

    public:
        // Constructor accepts Database object and path to search
        // Doesn't actually do anything yet
        LibraryScanner(const SyncDatabase &, const std::string &);

        // Prepare lists of files to add/edit/remove from database
        Status processFiles();

        // Process metadata for each required file
        // Accepts references to variables to update status
        // (current file, total files, estimated remaining time (secs))
        Status processMetadata(std::atomic<int> &, std::atomic<int> &, std::atomic<size_t> &);

        // Update the database with new data
        // !! Assumes that the database is locked for writing before calling !!
        Status updateDatabase();
};

#endif