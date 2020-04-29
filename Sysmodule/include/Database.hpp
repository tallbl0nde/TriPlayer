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
        // Opens read-only
        Database();

        // Return a path matching given ID (or blank if not found)
        std::string getPathForID(SongID);

        // Closes connection
        ~Database();
};

#endif