#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <sqlite3.h>
#include <vector>

// A songID is an int!
typedef int SongID;

// Struct storing information about song
struct SongInfo {
    SongID ID;              // unique ID
    std::string title;      // title
    std::string artist;     // artist name
    std::string album;      // album name
    unsigned int duration;  // in seconds
};

class Database {
    private:
        // sqlite3 handle
        sqlite3 * db;

        // sqlite3 statement handle
        sqlite3_stmt * cmd;

        // Creates database with empty tables
        void createTables();

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