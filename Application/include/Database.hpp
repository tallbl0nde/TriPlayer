#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "sqlite3.h"
#include <string>
#include "Types.hpp"
#include <vector>

// The Database class interacts with the database stored on the sd card
// to read/write data. It also caches some data in order to speed up
// some repeated queries.
class Database {
    private:
        // sqlite3 handle
        sqlite3 * db;

        // sqlite3 statement handle
        sqlite3_stmt * cmd;

        // Creates database with empty tables
        void createTables();

        // Call all necessary pragma statements
        void setup();

        // Log memory usage
        void logMemory();

        // Private queries
        void addArtist(std::string);
        void addAlbum(std::string, std::string);

    public:
        // Constructor opens (or creates) database
        Database();

        // Lock the database for writing (will block until writable)
        // Closes read-only connection and opens r/w
        void lock();

        // Unlock the database
        // Closes r/w connection and opens read-only
        void unlock();

        // Add song into database (handles artists, etc...)
        // Takes SongInfo, path and modified timestamp
        void addSong(SongInfo, std::string, unsigned int);
        // Update the given song's info and modified timestamp
        void updateSong(SongID, SongInfo, unsigned int);
        // Remove song from database with ID
        void removeSong(SongID);

        // Returns SongInfo for all stored songs
        std::vector<SongInfo> getAllSongInfo();
        // Returns vector of paths for all stored songs
        std::vector<std::string> getAllSongPaths();
        // Returns modified time for song matching path (0 on error/not found!)
        unsigned int getModifiedTimeForPath(std::string);
        // Return a path matching given ID (or blank if not found)
        std::string getPathForID(SongID);
        // Return ID of song with given path (-1 if not found)
        SongID getSongIDForPath(std::string);
        // Returns SongInfo for given ID (id will be -1 if not found!)
        SongInfo getSongInfoForID(SongID);

        // Cleans up database by:
        // Removing 'empty' artists/albums
        void cleanup();

        // Destructor closes handle
        ~Database();
};

#endif