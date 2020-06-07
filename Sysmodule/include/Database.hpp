#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "sqlite3.h"
#include <string>
#include "Types.hpp"

// Integrates with sqlite3 database to get required info for playback
class Database {
    private:
        // DB handle
        sqlite3 * db;

        // Statement object
        sqlite3_stmt * cmd;

    public:
        // Does nothing
        Database();

        // Opens a connection to database (returns true if successful)
        bool openConnection();
        // Drops an open connection to the database
        void dropConnection();
        // Returns true if connection is open and ready for queries
        bool ready();

        // Return a path matching given ID (or blank if not found)
        std::string getPathForID(SongID);

        // Closes connection
        ~Database();
};

#endif