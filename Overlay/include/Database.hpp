#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>

struct Metadata {
    int id;
    std::string title;
    std::string artist;
    unsigned int duration;
    std::string imagePath;
};

class SQLite;

typedef int SongID;

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
        // Constructor initializes database object
        Database();

        // Open the database read-write (will block until available)
        bool openReadOnly();
        // Close a open connection (if there is one)
        void close();

        // Return metadata for the given song (ID negative on error)
        Metadata getMetadataForID(SongID);

        // Destructor closes handle
        ~Database();
};

#endif