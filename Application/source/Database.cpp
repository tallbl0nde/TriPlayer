#include "Database.hpp"
#include "FS.hpp"
#include "Log.hpp"

// Location of backup file
#define BACKUP_PATH "/switch/TriPlayer/data_old.sqlite3"
// Location of the database file
#define DB_PATH "/switch/TriPlayer/data.sqlite3"
// Version of the database (database begins with zero from 'template', so this started at 1)
#define DB_VERSION 1
// Location of template file
#define TEMPLATE_DB_PATH "romfs:/Template.db"

// Custom boolean 'operator' which instead of 'keeping' true, will 'keep' false
bool keepFalse(const bool & a, const bool & b) {
    return !(!a || !b);
}

Database::Database() {
    // Copy the template if the database doesn't exist
    // Needed as SQLite spits IO errors otherwise
    if (!Utils::Fs::fileExists(DB_PATH)) {
        if (!Utils::Fs::copyFile(TEMPLATE_DB_PATH, DB_PATH)) {
            Log::writeError("[DB] Fatal error: unable to copy template database");
        }
    }

    // Create the database object
    this->db = new SQLite(DB_PATH);
    this->db->ignoreConstraints(true);
}

std::string Database::error() {
    return this->error_;
}

bool Database::migrate() {
    // Open read write
    bool ok = this->openReadWrite();
    if (!ok) {
        this->setErrorMsg("Unable to migrate the database as it is unwritable");
        return false;
    }

    // Check the version
    int version;
    ok = this->getVersion(version);
    if (ok) {
        if (version > DB_VERSION) {
            this->setErrorMsg("Database version is newer than what is supported (Database: " + std::to_string(version) + ", Supports: " + std::to_string(DB_VERSION) + ")");
            ok = false;

        // Don't proceed if the version matches
        } else if (version == DB_VERSION) {
            Log::writeInfo("[DB] Database version matches!");
            this->close();
            return true;
        }
    }

    // Otherwise let's first backup the current database
    if (version > 0 && ok) {
        this->close();
        if (!Utils::Fs::copyFile(DB_PATH, BACKUP_PATH)) {
            this->setErrorMsg("An error occurred backing up the database");
            return false;
        } else {
            Log::writeSuccess("[DB] Successfully backed up database");
        }
        this->openReadWrite();
    }

    // Now actually migrate
    if (ok) {
        switch (version) {
            case 0:
                ok = this->migrateTo1();
                if (!ok) { break; }
                Log::writeSuccess("[DB] Migrated to version 1");
                version++;
        }
    }

    // Log outcome and close database
    if (ok) {
        Log::writeSuccess("[DB] Migrations completed successfully!");
    } else {
        this->setErrorMsg("An error occurred migrating the database");
    }
    this->close();
    return ok;
}

// ======= Migrations ======= //
// Ver 1: Create all initial tables
bool Database::migrateTo1() {
    // Create Artists table
    bool ok = this->db->prepareQuery("CREATE TABLE Artists (id INTEGER NOT NULL PRIMARY KEY, name TEXT UNIQUE NOT NULL);");
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("Migration 1: Unable to create the Artists table");
        return false;
    }

    // Create Albums table
    ok = this->db->prepareQuery("CREATE TABLE Albums (id INTEGER NOT NULL PRIMARY KEY, name TEXT UNIQUE NOT NULL);");
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("Migration 1: Unable to create the Albums table");
        return false;
    }

    // Create Songs table
    ok = this->db->prepareQuery("CREATE TABLE Songs (id INTEGER NOT NULL PRIMARY KEY, path TEXT UNIQUE NOT NULL, modified DATETIME NOT NULL, lastplayed DATETIME NOT NULL DEFAULT 0, artist_id INT NOT NULL, album_id INT NOT NULL, title TEXT NOT NULL, duration INT NOT NULL, plays INT NOT NULL DEFAULT 0, favourite BOOLEAN NOT NULL DEFAULT 0, FOREIGN KEY (album_id) REFERENCES Albums (id), FOREIGN KEY (artist_id) REFERENCES Artists (id));");
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("Migration 1: Unable to create the Songs table");
        return false;
    }

    // Create Playlists table
    ok = this->db->prepareQuery("CREATE TABLE Playlists (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL, description TEXT NOT NULL DEFAULT \"\");");
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("Migration 1: Unable to create the Playlists table");
        return false;
    }

    // Create PlaylistSongs table
    ok = this->db->prepareQuery("CREATE TABLE PlaylistSongs (playlist_id INTEGER, song_id INTEGER, FOREIGN KEY (playlist_id) REFERENCES Playlists (id) ON DELETE CASCADE, FOREIGN KEY (song_id) REFERENCES Songs (id) ON DELETE CASCADE);");
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("Migration 1: Unable to create the Playlists table");
        return false;
    }

    // Insert new version number
    ok = this->db->prepareQuery("DELETE FROM Version;");
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("Migration 1: Unable to delete old version");
        return false;
    }
    ok = this->db->prepareQuery("INSERT INTO Version (number) VALUES (1);");
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("Migration 1: Unable to set version to 1");
        return false;
    }

    return true;
}
// ========================== //

void Database::setErrorMsg(const std::string & msg = "") {
    // Set error message to provided one
    if (msg.length() > 0) {
        this->error_ = msg;
        Log::writeError("[DB] " + msg);

    // Otherwise read latest from SQLite
    } else {
        this->error_ = this->db->errorMsg();
    }
}

bool Database::addArtist(std::string & name) {
    // Don't need to check for R/W as the callee will have done that
    bool ok = this->db->prepareQuery("INSERT INTO Artists (name) VALUES (?);");
    if (ok) {
        ok = this->db->bindString(0, name);
    }
    if (ok) {
        ok = this->db->executeQuery();
    }

    // Update error if there was one
    if (!ok) {
        this->setErrorMsg("An error occurred adding the artist '" + name + "'!");
    }
    return ok;
}

bool Database::addAlbum(std::string & name) {
    // Don't need to check for R/W as the callee will have done that
    bool ok = this->db->prepareQuery("INSERT INTO Albums (name) VALUES (?);");
    if (ok) {
        ok = this->db->bindString(0, name);
    }
    if (ok) {
        ok = this->db->executeQuery();
    }

    // Update error if there was one
    if (!ok) {
        this->setErrorMsg("An error occurred adding the album '" + name + "'!");
    }
    return ok;
}

bool Database::getVersion(int & version) {
    bool ok = this->db->prepareQuery("SELECT * FROM Version;");
    if (ok) {
        ok = this->db->executeQuery();
    }
    if (ok) {
        ok = this->db->getInt(0, version);
    }

    // Update error if there was one
    if (!ok) {
        this->setErrorMsg("Unable to get the database's version");
    }
    return ok;
}

// ============================ //
//       PUBLIC FUNCTIONS       //
// ============================ //
bool Database::openReadWrite() {
    return this->db->openConnection(SQLite::Connection::ReadWrite);
}

bool Database::openReadOnly() {
    return this->db->openConnection(SQLite::Connection::ReadOnly);
}

void Database::close() {
    this->db->closeConnection();
}

bool Database::addSong(SongInfo si, std::string & path, unsigned int mtime) {
    // First check we have write permission
    if (this->db->connectionType() != SQLite::Connection::ReadWrite) {
        this->setErrorMsg("[addSong] Can't add a song as the database is unwritable");
        return false;
    }

    // Add artist and album (will do nothing if they already exist)
    bool ok = this->addArtist(si.artist);
    ok = keepFalse(ok, this->addAlbum(si.album));
    if (!ok) {
        return false;
    }

    // Finally add song
    ok = this->db->prepareQuery("INSERT INTO Songs (path, modified, artist_id, album_id, title, duration) VALUES (?, ?, (SELECT id FROM Artists WHERE name = ?), (SELECT id FROM Albums WHERE name = ?), ?, ?);");
    ok = keepFalse(ok, this->db->bindString(0, path));
    ok = keepFalse(ok, this->db->bindInt(1, mtime));
    ok = keepFalse(ok, this->db->bindString(2, si.artist));
    ok = keepFalse(ok, this->db->bindString(3, si.album));
    ok = keepFalse(ok, this->db->bindString(4, si.title));
    ok = keepFalse(ok, this->db->bindInt(5, si.duration));
    if (!ok) {
        this->setErrorMsg("[addSong] An error occurred while preparing the statement");
        return false;
    }

    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[addSong] An error occurred while adding the entry");
    } else {
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("[DB] [addSong] '" + path + "' added to the database");
        }
    }

    return ok;
}

bool Database::updateSong(SongID id, SongInfo si, unsigned int mtime) {
    // First check we have write permission
    if (this->db->connectionType() != SQLite::Connection::ReadWrite) {
        this->setErrorMsg("[updateSong] Can't update song as the database is unwritable");
        return false;
    }

    // Add artist and album (will do nothing if they already exist)
    bool ok = this->addArtist(si.artist);
    ok = keepFalse(ok, this->addAlbum(si.album));
    if (!ok) {
        return false;
    }

    // Now update relevant fields
    ok = this->db->prepareQuery("UPDATE Songs SET modified = ?, artist_id = (SELECT id FROM Artists WHERE name = ?), album_id = (SELECT id FROM Albums WHERE name = ?), title = ?, duration = ? WHERE id = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, mtime));
    ok = keepFalse(ok, this->db->bindString(1, si.artist));
    ok = keepFalse(ok, this->db->bindString(2, si.album));
    ok = keepFalse(ok, this->db->bindString(3, si.title));
    ok = keepFalse(ok, this->db->bindInt(4, si.duration));
    ok = keepFalse(ok, this->db->bindInt(5, id));
    if (!ok) {
        this->setErrorMsg("[updateSong] An error occurred while preparing the statement");
        return false;
    }

    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[updateSong] An error occurred while adding the entry");
    } else {
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("[DB] [updateSong] '" + si.title + "' was updated");
        }
    }

    return ok;
}

bool Database::removeSong(SongID id) {
    // First check we have write permission
    if (this->db->connectionType() != SQLite::Connection::ReadWrite) {
        this->setErrorMsg("[removeSong] Can't remove song as the database is unwritable");
        return false;
    }

    bool ok = this->db->prepareQuery("DELETE FROM Songs WHERE id = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    if (!ok) {
        this->setErrorMsg("[removeSong] An error occurred while preparing the statement");
        return false;
    }

    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[removeSong] An error occurred while removing the entry");
    } else {
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("[DB] [removeSong] '" + std::to_string(id) + "' was deleted");
        }
    }

    return ok;
}

std::vector<SongInfo> Database::getAllSongInfo() {
    std::vector<SongInfo> v;
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getAllSongInfo] No open connection");
        return v;
    }

    // Create a SongInfo for each entry
    bool ok = this->db->prepareQuery("SELECT Songs.ID, Songs.title, Artists.name, Albums.name, Songs.duration FROM Songs JOIN Albums ON Albums.id = Songs.album_id JOIN Artists ON Artists.id = Songs.artist_id;");
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getAllSongInfo] Unable to query for all songs");
        return v;
    }
    while (ok) {
        SongInfo si;
        int tmp;
        ok = this->db->getInt(0, si.ID);
        ok = keepFalse(ok, this->db->getString(1, si.title));
        ok = keepFalse(ok, this->db->getString(2, si.artist));
        ok = keepFalse(ok, this->db->getString(3, si.album));
        ok = keepFalse(ok, this->db->getInt(4, tmp));
        si.duration = tmp;
        if (ok) {
            v.push_back(si);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    v.shrink_to_fit();
    return v;
}

std::vector<std::string> Database::getAllSongPaths() {
    std::vector<std::string> v;
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getAllSongPaths] No open connection");
        return v;
    }

    // Create a SongInfo for each entry
    bool ok = this->db->prepareQuery("SELECT path FROM Songs;");
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getAllSongPaths] Unable to query for all songs");
        return v;
    }
    while (ok) {
        std::string path;
        ok = this->db->getString(0, path);
        if (ok) {
            v.push_back(path);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    v.shrink_to_fit();
    return v;
}

unsigned int Database::getModifiedTimeForPath(std::string & path) {
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getModifiedTimeForPath] No open connection");
        return 0;
    }

    // Query modified time
    int time = 0;
    bool ok = this->db->prepareQuery("SELECT modified FROM Songs WHERE path = ?;");
    ok = keepFalse(ok, this->db->bindString(0, path));
    ok = keepFalse(ok, this->db->executeQuery());
    if (ok && this->db->hasRow()) {
        ok = this->db->getInt(0, time);
    }
    if (!ok) {
        this->setErrorMsg("[getModifiedTimeForPath] An error occurred querying modified time");
        return 0;
    }

    return time;
}

std::string Database::getPathForID(SongID id) {
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getPathForID] No open connection");
        return "";
    }

    // Query path
    std::string path;
    bool ok = this->db->prepareQuery("SELECT path FROM Songs WHERE id = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    ok = keepFalse(ok, this->db->getString(0, path));
    if (!ok) {
        this->setErrorMsg("[getPathForID] An error occurred querying the path");
        return "";
    }

    path.shrink_to_fit();
    return path;
}

SongID Database::getSongIDForPath(std::string & path) {
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getSongIDForPath] No open connection");
        return -1;
    }

    // Query ID
    SongID id;
    bool ok = this->db->prepareQuery("SELECT id FROM Songs WHERE path = ?;");
    ok = keepFalse(ok, this->db->bindString(0, path));
    ok = keepFalse(ok, this->db->executeQuery());
    ok = keepFalse(ok, this->db->getInt(0, id));
    if (!ok) {
        this->setErrorMsg("[getSongIDForPath] An error occurred querying path");
        return -1;
    }

    return id;
}

SongInfo Database::getSongInfoForID(SongID id) {
    SongInfo si;
    si.ID = -1;
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getSongInfoForID] No open connection");
        return si;
    }

    // Query for song info
    bool ok = this->db->prepareQuery("SELECT Songs.ID, Songs.title, Artists.name, Albums.name, Songs.duration FROM Songs JOIN Albums ON Albums.id = Songs.album_id JOIN Artists ON Artists.id = Songs.artist_id WHERE Songs.ID = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getSongInfoForID] An error occurred querying for info");
        return si;
    }
    ok = this->db->getInt(0, si.ID);
    ok = keepFalse(ok, this->db->getString(1, si.title));
    ok = keepFalse(ok, this->db->getString(2, si.artist));
    ok = keepFalse(ok, this->db->getString(3, si.album));
    int tmp;
    ok = keepFalse(ok, this->db->getInt(4, tmp));
    si.duration = tmp;
    if (!ok) {
        this->setErrorMsg("[getSongInfoForID] An error occurred reading from the query results");
        si.ID = -1;
    }

    return si;
}

void Database::cleanup() {

}

Database::~Database() {
    this->close();
}