#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "SQLite.hpp"
#include "Types.hpp"

// The Database class interacts with the database stored on the sd card
// to read/write data. All queries have a way of detecting if they failed.
class Database {
    private:
        // Interface to database
        SQLite * db;

        // Private queries
        bool getVersion(int &);
        bool matchingVersion();

    public:
        // Constructor creates + 'migrates' the database to a newer version if needed
        Database();

        // Open the database read-write (will block until available)
        bool openReadOnly();
        // Open the database read-only (will block until available)
        bool openReadWrite();
        // Close a open connection (if there is one)
        void close();

        // Return a path matching given ID (or blank if not found)
        std::string getPathForID(SongID);

        // Destructor closes handle
        ~Database();
};

#endif