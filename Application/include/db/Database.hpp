#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "SQLite.hpp"
#include "Types.hpp"
#include <vector>

// The Database class interacts with the database stored on the sd card
// to read/write data. All queries have a way of detecting if they failed.
// If so, error() will return a non-empty string describing the error.
class Database {
    private:
        // Interface to database
        SQLite * db;
        // String describing last error
        std::string error_;

        // Migrations - explanation in .cpp (all follow naming: migrateTo<version>)
        bool migrateTo1();

        // Update the stored error message
        void setErrorMsg(const std::string &);

        // Private queries
        bool addArtist(std::string &);
        bool addAlbum(std::string &);
        bool getVersion(int &);

    public:
        // Constructor creates + 'migrates' the database to a newer version if needed
        Database();
        // Migrates the database to bring it to the latest version
        bool migrate();

        // Returns the last error that occurred (blank if no error has occurred)
        std::string error();

        // Open the database read-write (will block until available)
        bool openReadOnly();
        // Open the database read-only (will block until available)
        bool openReadWrite();
        // Close a open connection (if there is one)
        void close();

        // Add song into database (handles artists, etc...)
        // Takes SongInfo, path and modified timestamp
        // Returns true if successful, false otherwise
        bool addSong(SongInfo, std::string &, unsigned int);
        // Update the given song's info and modified timestamp
        // Returns true if successful, false otherwise
        bool updateSong(SongID, SongInfo, unsigned int);
        // Remove song from database with ID
        // Returns true if successful, false otherwise
        bool removeSong(SongID);

        // Returns SongInfo for all stored songs
        // Empty if no songs or an error occurred
        std::vector<SongInfo> getAllSongInfo();
        // Returns vector of paths for all stored songs
        // Empty if no songs or an error occurred
        std::vector<std::string> getAllSongPaths();
        // Returns modified time for song matching path (0 on error/not found!)
        unsigned int getModifiedTimeForPath(std::string &);
        // Return a path matching given ID (or blank if not found)
        std::string getPathForID(SongID);
        // Return ID of song with given path (-1 if not found)
        SongID getSongIDForPath(std::string &);
        // Returns SongInfo for given ID (id will be -1 if not found!)
        SongInfo getSongInfoForID(SongID);

        // Tidies up database by removing redundant entries
        void cleanup();

        // Destructor closes handle
        ~Database();
};

#endif