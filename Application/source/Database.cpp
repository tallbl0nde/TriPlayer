#include "Database.hpp"
#include <filesystem>
#include <fstream>
#include "Log.hpp"

void Database::logMemory() {
    int mem;
    int himem;
    if (sqlite3_status(SQLITE_STATUS_MEMORY_USED, &mem, &himem, false) == SQLITE_OK) {
        Log::writeInfo("[SQLITE] Current memory usage: " + std::to_string(mem/1024) + "kB - Max usage: " + std::to_string(himem/1024) + "kB");
    } else {
        Log::writeInfo("[SQLITE] Unable to query memory usage!");
    }
}

Database::Database() {
    // Check if file exists, and if not create directories
    bool exists = true;
    if (!std::filesystem::exists("/switch/TriPlayer/music.db")) {
        std::filesystem::create_directory("/switch");
        std::filesystem::create_directory("/switch/TriPlayer");
        exists = false;

        // Copy 'template' file
        // Required as the file is unwritable otherwise
        std::ifstream srcF("romfs:/Template.db", std::ios::binary);
        std::ofstream destF("/switch/TriPlayer/music.db", std::ios::binary);
        destF << srcF.rdbuf();
        srcF.close();
        destF.flush();
        destF.close();
    }

    // Create database tables if database file was just copied
    if (!exists) {
        this->lock();
        this->createTables();
    }

    // Open read-only
    this->unlock();
    this->logMemory();
}

void Database::setup() {
    // Store journal in memory (otherwise file is unwritable)
    sqlite3_prepare_v2(this->db, "PRAGMA journal_mode=MEMORY;", -1, &this->cmd, nullptr);
    if (this->cmd != nullptr) {
        sqlite3_step(this->cmd);
        Log::writeSuccess("[SQLITE] Set journal_mode to MEMORY");
    } else {
        Log::writeError("[SQLITE] Unable to set journal_mode to MEMORY");
    }
    sqlite3_finalize(this->cmd);

    // Use foreign keys
    sqlite3_prepare_v2(this->db, "PRAGMA foreign_keys=ON;", -1, &this->cmd, nullptr);
    if (this->cmd != nullptr) {
        sqlite3_step(this->cmd);
        Log::writeSuccess("[SQLITE] Enabled foreign keys");
    } else {
        Log::writeError("[SQLITE] Unable to enable foreign keys");
    }
    sqlite3_finalize(this->cmd);
}

void Database::lock() {
    sqlite3_close(this->db);
    sqlite3_open_v2("/switch/TriPlayer/music.db", &this->db, SQLITE_OPEN_READWRITE, "unix-none");
    Log::writeInfo("[SQLITE] Open database read-write (lock)");
    this->setup();
}

void Database::unlock() {
    sqlite3_close(this->db);
    sqlite3_open_v2("/switch/TriPlayer/music.db", &this->db, SQLITE_OPEN_READONLY, "unix-none");
    Log::writeInfo("[SQLITE] Open database read-only (unlock)");
    this->setup();
}

void Database::createTables() {
    // Create Artists table
    sqlite3_prepare_v2(this->db, "CREATE TABLE Artists (id INTEGER NOT NULL PRIMARY KEY, name TEXT UNIQUE NOT NULL);", -1, &this->cmd, nullptr);
    Log::writeInfo("[SQLITE] Prepare 'create artists' statement");
    if (this->cmd != nullptr) {
        sqlite3_step(this->cmd);
        Log::writeInfo("[SQLITE] Create Artists table");
    }
    sqlite3_finalize(this->cmd);

    // Create Albums table
    sqlite3_prepare_v2(this->db, "CREATE TABLE Albums (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL, artist_id INTEGER NOT NULL, FOREIGN KEY (artist_id) REFERENCES Artists (id));", -1, &this->cmd, nullptr);
    Log::writeInfo("[SQLITE] Prepare 'create albums' statement");
    if (this->cmd != nullptr) {
        sqlite3_step(this->cmd);
        Log::writeInfo("[SQLITE] Create Albums table");
    }
    sqlite3_finalize(this->cmd);

    // Create Songs table
    sqlite3_prepare_v2(this->db, "CREATE TABLE Songs (id INTEGER NOT NULL PRIMARY KEY, path TEXT UNIQUE NOT NULL, modified DATETIME NOT NULL, lastplayed DATETIME NOT NULL DEFAULT 0, album_id INT NOT NULL, title TEXT NOT NULL, duration INT NOT NULL, plays INT NOT NULL DEFAULT 0, favourite BOOLEAN NOT NULL DEFAULT 0, FOREIGN KEY (album_id) REFERENCES Albums (id) );", -1, &this->cmd, nullptr);
    Log::writeInfo("[SQLITE] Prepare 'create songs' statement");
    if (this->cmd != nullptr) {
        sqlite3_step(this->cmd);
        Log::writeInfo("[SQLITE] Create Songs table");
    }
    sqlite3_finalize(this->cmd);

    // Create Playlists table
    sqlite3_prepare_v2(this->db, "CREATE TABLE Playlists (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL, description TEXT NOT NULL DEFAULT \"\");", -1, &this->cmd, nullptr);
    Log::writeInfo("[SQLITE] Prepare 'create playlists' statement");
    if (this->cmd != nullptr) {
        sqlite3_step(this->cmd);
        Log::writeInfo("[SQLITE] Create Playlists table");
    }
    sqlite3_finalize(this->cmd);

    // Create PlaylistSongs table
    sqlite3_prepare_v2(this->db, "CREATE TABLE PlaylistSongs (playlist_id INTEGER, song_id INTEGER, FOREIGN KEY (playlist_id) REFERENCES Playlists (id) ON DELETE CASCADE, FOREIGN KEY (song_id) REFERENCES Songs (id) ON DELETE CASCADE);", -1, &this->cmd, nullptr);
    Log::writeInfo("[SQLITE] Prepare 'create playlistsongs' statement");
    if (this->cmd != nullptr) {
        sqlite3_step(this->cmd);
        Log::writeInfo("[SQLITE] Create PlaylistSongs table");
    }
    sqlite3_finalize(this->cmd);
}

void Database::addArtist(std::string name) {
    sqlite3_prepare_v2(this->db, "INSERT INTO Artists (name) VALUES (?);", -1, &this->cmd, nullptr);
    sqlite3_bind_text(this->cmd, 1, name.c_str(), -1, SQLITE_STATIC);
    Log::writeInfo("[SQLITE] Prepare 'add artist' statement");
    if (this->cmd != nullptr) {
        sqlite3_step(this->cmd);
        Log::writeInfo("[SQLITE] Add artist");
    }
    sqlite3_finalize(this->cmd);
}

void Database::addAlbum(std::string name, std::string artist) {
    sqlite3_prepare_v2(this->db, "INSERT INTO Albums (name, artist_id) VALUES (?, (SELECT id FROM Artists WHERE name = ?));", -1, &this->cmd, nullptr);
    sqlite3_bind_text(this->cmd, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(this->cmd, 2, artist.c_str(), -1, SQLITE_STATIC);
    Log::writeInfo("[SQLITE] Prepare 'add album' statement");
    if (this->cmd != nullptr) {
        sqlite3_step(this->cmd);
        Log::writeInfo("[SQLITE] Add album");
    }
    sqlite3_finalize(this->cmd);
}

void Database::addSong(SongInfo si, std::string path, unsigned int mtime) {
    // Add artist first (will do nothing if they already exist)
    this->addArtist(si.artist);

    // Add album next
    this->addAlbum(si.album, si.artist);

    // Finally add song
    sqlite3_prepare_v2(this->db, "INSERT INTO Songs (path, modified, album_id, title, duration) VALUES (?, ?, (SELECT id FROM Albums WHERE name = ?), ?, ?);", -1, &this->cmd, nullptr);
    sqlite3_bind_text(this->cmd, 1, path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(this->cmd, 2, mtime);
    sqlite3_bind_text(this->cmd, 3, si.album.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(this->cmd, 4, si.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(this->cmd, 5, si.duration);
    Log::writeInfo("[SQLITE] Prepare 'add song' statement");
    if (this->cmd != nullptr) {
        sqlite3_step(this->cmd);
        Log::writeInfo("[SQLITE] Add song");
    }
    sqlite3_finalize(this->cmd);
}

void Database::updateSong(SongID id, SongInfo si, unsigned int mtime) {
    // Ensure artist and album exist
    this->addArtist(si.artist);
    this->addAlbum(si.album, si.artist);

    // Now update relevant fields
    sqlite3_prepare_v2(this->db, "UPDATE Songs SET modified = ?, album_id = (SELECT id FROM Albums WHERE name = ?), title = ?, duration = ? WHERE id = ?;", -1, &this->cmd, nullptr);
    sqlite3_bind_int(this->cmd, 1, mtime);
    sqlite3_bind_text(this->cmd, 2, si.album.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(this->cmd, 3, si.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(this->cmd, 4, si.duration);
    sqlite3_bind_int(this->cmd, 5, id);
    Log::writeInfo("[SQLITE] Prepare 'update song' statement");
    if (this->cmd != nullptr) {
        sqlite3_step(this->cmd);
        Log::writeInfo("[SQLITE] Update song");
    }
    sqlite3_finalize(this->cmd);
}

void Database::removeSong(SongID id) {
    sqlite3_prepare_v2(this->db, "DELETE FROM Songs WHERE id = ?;", -1, &this->cmd, nullptr);
    sqlite3_bind_int(this->cmd, 1, id);
    Log::writeInfo("[SQLITE] Prepare 'remove song' statement");
    if (this->cmd != nullptr) {
        sqlite3_step(this->cmd);
        Log::writeInfo("[SQLITE] Remove song");
    }
    sqlite3_finalize(this->cmd);
}

std::vector<SongInfo> Database::getAllSongInfo() {
    std::vector<SongInfo> v;

    // Set size of vector based on number of songs
    sqlite3_prepare_v2(this->db, "SELECT COUNT(*) FROM Songs;", -1, &this->cmd, nullptr);
    Log::writeInfo("[SQLITE] Prepare 'song count' statement");
    if (this->cmd != nullptr) {
        int rows = -1;
        if (sqlite3_step(this->cmd) == SQLITE_ROW) {
            rows = sqlite3_column_int(this->cmd, 0);
            v.resize(rows);
        }
        Log::writeInfo("[SQLITE] Got count (" + std::to_string(rows) + ")");
    }
    sqlite3_finalize(this->cmd);

    // Create a SongInfo for each entry
    sqlite3_prepare_v2(this->db, "SELECT Songs.ID, Songs.title, Artists.name, Albums.name, Songs.duration FROM Songs JOIN Albums ON Albums.id = Songs.album_id JOIN Artists ON Artists.id = Albums.artist_id;", -1, &this->cmd, nullptr);
    Log::writeInfo("[SQLITE] Prepare 'select all songs' statement");
    if (this->cmd != nullptr) {
        unsigned int pos = 0;
        while (sqlite3_step(this->cmd) == SQLITE_ROW) {
            SongInfo i;
            i.ID = sqlite3_column_int(this->cmd, 0);
            const unsigned char * str = sqlite3_column_text(this->cmd, 1);
            i.title = std::string((const char *)str);
            str = sqlite3_column_text(this->cmd, 2);
            i.artist = std::string((const char *)str);
            str = sqlite3_column_text(this->cmd, 3);
            i.album = std::string((const char *)str);
            i.duration = sqlite3_column_int(this->cmd, 4);
            v[pos] = i;
            pos++;
        }
    }
    sqlite3_finalize(this->cmd);

    return v;
}

std::vector<std::string> Database::getAllSongPaths() {
    std::vector<std::string> v;

    // Set size of vector based on number of songs
    sqlite3_prepare_v2(this->db, "SELECT COUNT(*) FROM Songs;", -1, &this->cmd, nullptr);
    Log::writeInfo("[SQLITE] Prepare 'song count' statement");
    if (this->cmd != nullptr) {
        int rows = -1;
        if (sqlite3_step(this->cmd) == SQLITE_ROW) {
            rows = sqlite3_column_int(this->cmd, 0);
            v.resize(rows);
        }
        Log::writeInfo("[SQLITE] Got count (" + std::to_string(rows) + ")");
    }
    sqlite3_finalize(this->cmd);

    // Get each path and insert in vector
    sqlite3_prepare_v2(this->db, "SELECT path FROM Songs;", -1, &this->cmd, nullptr);
    Log::writeInfo("[SQLITE] Prepare 'select all song paths' statement");
    if (this->cmd != nullptr) {
        unsigned int pos = 0;
        while (sqlite3_step(this->cmd) == SQLITE_ROW) {
            const unsigned char * str = sqlite3_column_text(this->cmd, 0);
            v[pos] = std::string((const char *)str);
            pos++;
        }
        Log::writeInfo("[SQLITE] Finish iterating over rows");
    }
    sqlite3_finalize(this->cmd);

    return v;
}

unsigned int Database::getModifiedTimeForPath(std::string path) {
    unsigned int ret = 0;

    sqlite3_prepare_v2(this->db, "SELECT modified FROM Songs WHERE path = ?;", -1, &this->cmd, nullptr);
    sqlite3_bind_text(this->cmd, 1, path.c_str(), -1, SQLITE_STATIC);
    Log::writeInfo("[SQLITE] Prepare statement");
    if (this->cmd != nullptr) {
        if (sqlite3_step(this->cmd) == SQLITE_ROW) {
            ret = sqlite3_column_int(this->cmd, 0);
        }
    }
    sqlite3_finalize(this->cmd);

    return ret;
}

std::string Database::getPathForID(SongID id) {
    std::string str = "";

    sqlite3_prepare_v2(this->db, "SELECT path FROM Songs WHERE id = ?;", -1, &this->cmd, nullptr);
    if (this->cmd != nullptr) {
        sqlite3_bind_int(this->cmd, 1, id);
        if (sqlite3_step(this->cmd) == SQLITE_ROW) {
            const unsigned char * s = sqlite3_column_text(this->cmd, 0);
            str = std::string((const char *)s);
        }
    }
    sqlite3_finalize(cmd);

    return str;
}

SongID Database::getSongIDForPath(std::string path) {
    SongID ret = -1;
    sqlite3_prepare_v2(this->db, "SELECT id FROM Songs WHERE path = ?;", -1, &this->cmd, nullptr);
    sqlite3_bind_text(this->cmd, 1, path.c_str(), -1, SQLITE_STATIC);
    Log::writeInfo("[SQLITE] Prepare statement");
    if (this->cmd != nullptr) {
        if (sqlite3_step(this->cmd) == SQLITE_ROW) {
            ret = sqlite3_column_int(this->cmd, 0);
        }
    }
    sqlite3_finalize(this->cmd);

    return ret;
}

SongInfo Database::getSongInfoForID(SongID id) {
    SongInfo i;
    i.ID = -1;

    std::string s = "SELECT Songs.ID, Songs.title, Artists.name, Albums.name, Songs.duration FROM Songs JOIN Albums ON Albums.id = Songs.album_id JOIN Artists ON Artists.id = Albums.artist_id WHERE Songs.ID = " + std::to_string(id) + ";";
    sqlite3_prepare_v2(this->db, s.c_str(), -1, &this->cmd, nullptr);
    Log::writeInfo("[SQLITE] Prepare statement");
    if (this->cmd != nullptr) {
        if (sqlite3_step(this->cmd) == SQLITE_ROW) {
            i.ID = sqlite3_column_int(this->cmd, 0);
            const unsigned char * str = sqlite3_column_text(this->cmd, 1);
            i.title = std::string((const char *)str);
            str = sqlite3_column_text(this->cmd, 2);
            i.artist = std::string((const char *)str);
            str = sqlite3_column_text(this->cmd, 3);
            i.album = std::string((const char *)str);
            i.duration = sqlite3_column_int(this->cmd, 4);
        }
        Log::writeInfo("[SQLITE] Read column values");
    }
    sqlite3_finalize(this->cmd);

    return i;
}

void Database::cleanup() {

}

Database::~Database() {
    sqlite3_close(this->db);
}