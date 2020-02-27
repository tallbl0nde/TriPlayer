#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "sqlite3.h"
#include <string>
#include "Types.hpp"
#include <vector>

class Database {
    private:
        // sqlite3 handle
        sqlite3 * db;

        // sqlite3 statement handle
        sqlite3_stmt * cmd;

        // Creates database with empty tables
        void createTables();

        // Log given string alond with extended code (to stdout)
        // Format: "[SQLITE] string: status (code)"
        void logMessage(std::string);

        // Log memory usage
        void logMemory();

    public:
        // Constructor opens (or creates) database
        Database();

        // Returns SongInfo for all stored songs
        std::vector<SongInfo> getAllSongInfo();
        // Returns SongInfo for given ID (id will be -1 if not found!)
        SongInfo getSongInfoForID(SongID);

        // Destructor closes handle
        ~Database();
};

#endif