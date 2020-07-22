#include "db/Database.hpp"
#include "db/migrations/Migration.hpp"
#include "db/Spellfix.h"
#include "Log.hpp"
#include "utils/FS.hpp"
#include "utils/Utils.hpp"

// Location of backup file
#define BACKUP_PATH "/switch/TriPlayer/data_old.sqlite3"
// Location of the database file
#define DB_PATH "/switch/TriPlayer/data.sqlite3"
// Version of the database (database begins with zero from 'template', so this started at 1)
#define DB_VERSION 3
// Location of template file
#define TEMPLATE_DB_PATH "romfs:/db/template.sqlite3"

// Custom boolean 'operator' which instead of 'keeping' true, will 'keep' false
bool keepFalse(const bool & a, const bool & b) {
    return !(!a || !b);
}

// ===== Housekeeping ===== //
Database::Database() {
    // Copy the template if the database doesn't exist
    // Needed as SQLite spits IO errors otherwise
    if (!Utils::Fs::fileExists(DB_PATH)) {
        if (!Utils::Fs::copyFile(TEMPLATE_DB_PATH, DB_PATH)) {
            Log::writeError("[DB] Fatal error: unable to copy template database");
        } else {
            Log::writeSuccess("[DB] Copied template database successfully");
        }
    }

    // Create the database object
    this->db = new SQLite(DB_PATH);
    this->db->ignoreConstraints(true);

    // Load the spellfix1 extension
    sqlite3_auto_extension((void (*)(void))sqlite3_spellfix_init);

    // Set variables
    this->error_ = "";
    this->updateMarked = false;
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
    std::string err = "";
    if (ok) {
        switch (version) {
            case 0:
                err = Migration::migrateTo1(this->db);
                if (!err.empty()) {
                    err = "Migration 1: " + err;
                    break;
                }
                Log::writeSuccess("[DB] Migrated to version 1");

            case 1:
                err = Migration::migrateTo2(this->db);
                if (!err.empty()) {
                    err = "Migration 2: " + err;
                    break;
                }
                Log::writeSuccess("[DB] Migrated to version 2");

            case 2:
                err = Migration::migrateTo3(this->db);
                if (!err.empty()) {
                    err = "Migration 3: " + err;
                    break;
                }
                Log::writeSuccess("[DB] Migrated to version 3");
        }
    }

    // Log outcome and close database
    if (err.empty()) {
        Log::writeSuccess("[DB] Migrations completed successfully!");
    } else {
        this->setErrorMsg(err);
        this->setErrorMsg("An error occurred migrating the database");
    }
    this->close();
    return ok;
}

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

// ===== Private Queries ===== //
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
    bool ok = this->db->prepareQuery("SELECT value FROM Variables WHERE name = 'version';");
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

bool Database::setSearchUpdate(int val) {
    // Skip if already set
    if (this->updateMarked && val != 0) {
        return true;
    }

    bool ok = this->db->prepareQuery("UPDATE Variables SET value = ? WHERE name = 'search_update';");
    ok = keepFalse(ok, this->db->bindInt(0, val));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[setSearchUpdate] Updating the search_update variable failed");
    }

    // Set as marked if not 0
    if (ok) {
        this->updateMarked = (val != 0);
    }

    return ok;
}

bool Database::spellfixString(const std::string & table, std::string & str) {
    // Split into words
    std::vector<std::string> words = Utils::splitIntoWords(str);
    str = "";

    // Fix each word using the spellfix extension
    for (size_t i = 0; i < words.size(); i++) {
        // Use string concatenation here as you can't bind a table name (I know it's not ideal but the names are hard coded at least)
        bool ok = this->db->prepareQuery("SELECT word FROM " + table + " WHERE word MATCH ?;");
        ok = keepFalse(ok, this->db->bindString(0, words[i]));
        ok = keepFalse(ok, this->db->executeQuery());
        if (!ok) {
            this->setErrorMsg("Error performing spell check");
            return false;
        }

        // Add fixed word onto string
        std::string word;
        ok = this->db->getString(0, word);
        if (!ok) {
            this->setErrorMsg("Error performing spell check");
            return false;
        }
        str += word;
        if (i < words.size() - 1) {
            str += " ";
        }
    }

    return true;
}

// ===== Connection Management ===== //
bool Database::openReadWrite() {
    return this->db->openConnection(SQLite::Connection::ReadWrite);
}

bool Database::openReadOnly() {
    return this->db->openConnection(SQLite::Connection::ReadOnly);
}

void Database::close() {
    this->db->closeConnection();
}

// ===== Album Metadata ===== //
std::vector<Metadata::Album> Database::getAllAlbumMetadata() {
    std::vector<Metadata::Album> v;
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getAllAlbumMetadata] No open connection");
        return v;
    }

    // Create a Metadata::Album for each entry
    bool ok = this->db->prepareAndExecuteQuery("SELECT album_id, Albums.name, CASE WHEN COUNT(DISTINCT artist_id) > 1 THEN 'Various Artists' ELSE Artists.name END, Albums.tadb_id, Albums.image_path, COUNT(*) FROM Songs JOIN Albums ON Songs.album_id = Albums.id JOIN Artists ON Songs.artist_id = Artists.id GROUP BY album_id ORDER BY Albums.name;");
    if (!ok) {
        this->setErrorMsg("[getAllAlbumMetadata] Unable to query for all albums");
        return v;
    }
    while (ok) {
        Metadata::Album m;
        ok = this->db->getInt(0, m.ID);
        ok = keepFalse(ok, this->db->getString(1, m.name));
        ok = keepFalse(ok, this->db->getString(2, m.artist));
        ok = keepFalse(ok, this->db->getInt(3, m.tadbID));
        ok = keepFalse(ok, this->db->getString(4, m.imagePath));
        int tmp;
        ok = keepFalse(ok, this->db->getInt(5, tmp));
        m.songCount = tmp;

        if (ok) {
            v.push_back(m);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    return v;
}

Metadata::Album Database::getAlbumMetadataForID(AlbumID id) {
    Metadata::Album m;
    m.ID = -1;

    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getAlbumMetadataForID] No open connection");
        return m;
    }

    // Create a Metadata::Album
    bool ok = this->db->prepareQuery("SELECT album_id, Albums.name, CASE WHEN COUNT(DISTINCT artist_id) > 1 THEN 'Various Artists' ELSE Artists.name END, Albums.tadb_id, Albums.image_path, COUNT(*) FROM Songs JOIN Albums ON Songs.album_id = Albums.id JOIN Artists ON Songs.artist_id = Artists.id WHERE Songs.album_id = ? GROUP BY Songs.album_id;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getAlbumMetadataForID] An error occurred querying for info");
        return m;
    }
    int tmp;
    ok = this->db->getInt(0, m.ID);
    ok = keepFalse(ok, this->db->getString(1, m.name));
    ok = keepFalse(ok, this->db->getString(2, m.artist));
    ok = keepFalse(ok, this->db->getInt(3, m.tadbID));
    ok = keepFalse(ok, this->db->getString(4, m.imagePath));
    ok = keepFalse(ok, this->db->getInt(5, tmp));
    m.songCount = tmp;

    if (!ok) {
        this->setErrorMsg("[getAlbumMetadataForID] An error occurred reading from the query results");
        m.ID = -1;
    }

    return m;
}

std::vector<Metadata::Album> Database::getAlbumMetadataForArtist(ArtistID id) {
    std::vector<Metadata::Album> v;

    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getAlbumMetadataForArtist] No open connection");
        return v;
    }

    // Create a Metadata::Album (note this query won't ever return 'Various Artists' as the artist but that's alright seeing how we're querying for an artist)
    bool ok = this->db->prepareQuery("SELECT album_id, Albums.name, Artists.name, Albums.tadb_id, Albums.image_path, COUNT(*) FROM Songs JOIN Albums ON Songs.album_id = Albums.id JOIN Artists ON Songs.artist_id = Artists.id WHERE Songs.artist_id = ? GROUP BY Songs.album_id ORDER BY Albums.name;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getAlbumMetadataForArtist] Unable to query for artist's albums");
        return v;
    }
    while (ok) {
        Metadata::Album m;
        int tmp;
        ok = this->db->getInt(0, m.ID);
        ok = keepFalse(ok, this->db->getString(1, m.name));
        ok = keepFalse(ok, this->db->getString(2, m.artist));
        ok = keepFalse(ok, this->db->getInt(3, m.tadbID));
        ok = keepFalse(ok, this->db->getString(4, m.imagePath));
        ok = keepFalse(ok, this->db->getInt(5, tmp));
        m.songCount = tmp;

        if (ok) {
            v.push_back(m);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    return v;
}

// ===== Artist Metadata ===== //
bool Database::updateArtist(Metadata::Artist m) {
    // First check we have write permission
    if (this->db->connectionType() != SQLite::Connection::ReadWrite) {
        this->setErrorMsg("[updateArtist] Can't update song as the database is unwritable");
        return false;
    }

    // Now update relevant fields
    bool ok = this->db->prepareQuery("UPDATE Artists SET name = ?, tadb_id = ?, image_path = ? WHERE id = ?;");
    ok = keepFalse(ok, this->db->bindString(0, m.name));
    ok = keepFalse(ok, this->db->bindInt(1, m.tadbID));
    ok = keepFalse(ok, this->db->bindString(2, m.imagePath));
    ok = keepFalse(ok, this->db->bindInt(3, m.ID));
    if (!ok) {
        this->setErrorMsg("[updateArtist] An error occurred while preparing the statement");
        return false;
    }

    // We don't want to ignore constraints for this query
    this->db->ignoreConstraints(false);
    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[updateArtist] An error occurred while updating the entry");
    } else {
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("[DB] [updateArtist] '" + m.name + "' was updated");
        }
    }
    this->db->ignoreConstraints(true);

    // Mark search tables as out of date
    if (ok) {
        ok = this->setSearchUpdate(1);
    }

    return ok;
}

std::vector<Metadata::Artist> Database::getAllArtistMetadata() {
    std::vector<Metadata::Artist> v;
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getAllArtists] No open connection");
        return v;
    }

    // Create a Metadata::Artist for each entry
    bool ok = this->db->prepareAndExecuteQuery("SELECT artist_id, Artists.name, Artists.tadb_id, Artists.image_path, COUNT(DISTINCT album_id), COUNT(*) FROM Songs JOIN Artists ON Songs.artist_id = Artists.id GROUP BY artist_id ORDER BY Artists.name;");
    if (!ok) {
        this->setErrorMsg("[getAllArtists] Unable to query for all artists");
        return v;
    }
    while (ok) {
        Metadata::Artist m;
        ok = this->db->getInt(0, m.ID);
        ok = keepFalse(ok, this->db->getString(1, m.name));
        ok = keepFalse(ok, this->db->getInt(2, m.tadbID));
        ok = keepFalse(ok, this->db->getString(3, m.imagePath));
        int tmp;
        ok = keepFalse(ok, this->db->getInt(4, tmp));
        m.albumCount = tmp;
        ok = keepFalse(ok, this->db->getInt(5, tmp));
        m.songCount = tmp;

        if (ok) {
            v.push_back(m);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    return v;
}

std::vector<Metadata::Artist> Database::getArtistMetadataForAlbum(AlbumID id) {
    std::vector<Metadata::Artist> v;
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getArtistMetadataForAlbum] No open connection");
        return v;
    }

    // Create a Metadata::Artist for each entry (note this query won't ever return more than '1' as the number of albums as we're querying for a single album)
    bool ok = this->db->prepareQuery("SELECT artist_id, Artists.name, Artists.tadb_id, Artists.image_path, COUNT(DISTINCT album_id), COUNT(*) FROM Songs JOIN Artists ON Songs.artist_id = Artists.id WHERE Songs.album_id = ? GROUP BY artist_id ORDER BY Artists.name;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getArtistMetadataForAlbum] Unable to query for an album's artists");
        return v;
    }
    while (ok) {
        Metadata::Artist m;
        ok = this->db->getInt(0, m.ID);
        ok = keepFalse(ok, this->db->getString(1, m.name));
        ok = keepFalse(ok, this->db->getInt(2, m.tadbID));
        ok = keepFalse(ok, this->db->getString(3, m.imagePath));
        int tmp;
        ok = keepFalse(ok, this->db->getInt(4, tmp));
        m.albumCount = tmp;
        ok = keepFalse(ok, this->db->getInt(5, tmp));
        m.songCount = tmp;

        if (ok) {
            v.push_back(m);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    return v;
}

Metadata::Artist Database::getArtistMetadataForID(ArtistID id) {
    Metadata::Artist m;
    m.ID = -1;

    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getArtistMetadataForID] No open connection");
        return m;
    }

    // Create a Metadata::Artist for each entry
    bool ok = this->db->prepareQuery("SELECT artist_id, Artists.name, Artists.tadb_id, Artists.image_path, COUNT(DISTINCT album_id), COUNT(*) FROM Songs JOIN Artists ON Songs.artist_id = Artists.id WHERE Songs.artist_id = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getArtistMetadataForID] An error occurred querying for info");
        return m;
    }
    int tmp;
    ok = this->db->getInt(0, m.ID);
    ok = keepFalse(ok, this->db->getString(1, m.name));
    ok = keepFalse(ok, this->db->getInt(2, m.tadbID));
    ok = keepFalse(ok, this->db->getString(3, m.imagePath));
    ok = keepFalse(ok, this->db->getInt(4, tmp));
    m.albumCount = tmp;
    ok = keepFalse(ok, this->db->getInt(5, tmp));
    m.songCount = tmp;
    if (!ok) {
        this->setErrorMsg("[getArtistMetadataForID] An error occurred reading from the query results");
        m.ID = -1;
    }

    return m;
}

// ===== Song Metadata ===== //
bool Database::addSong(Metadata::Song m) {
    // First check we have write permission
    if (this->db->connectionType() != SQLite::Connection::ReadWrite) {
        this->setErrorMsg("[addSong] Can't add a song as the database is unwritable");
        return false;
    }

    // Add artist and album (will do nothing if they already exist)
    bool ok = this->addArtist(m.artist);
    ok = keepFalse(ok, this->addAlbum(m.album));
    if (!ok) {
        return false;
    }

    // Finally add song
    ok = this->db->prepareQuery("INSERT INTO Songs (path, modified, artist_id, album_id, title, duration, track, disc) VALUES (?, ?, (SELECT id FROM Artists WHERE name = ?), (SELECT id FROM Albums WHERE name = ?), ?, ?, ?, ?);");
    ok = keepFalse(ok, this->db->bindString(0, m.path));
    ok = keepFalse(ok, this->db->bindInt(1, m.modified));
    ok = keepFalse(ok, this->db->bindString(2, m.artist));
    ok = keepFalse(ok, this->db->bindString(3, m.album));
    ok = keepFalse(ok, this->db->bindString(4, m.title));
    ok = keepFalse(ok, this->db->bindInt(5, m.duration));
    ok = keepFalse(ok, this->db->bindInt(6, m.trackNumber));
    ok = keepFalse(ok, this->db->bindInt(7, m.discNumber));
    if (!ok) {
        this->setErrorMsg("[addSong] An error occurred while preparing the statement");
        return false;
    }

    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[addSong] An error occurred while adding the entry");
    } else {
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("[DB] [addSong] '" + m.path + "' added to the database");
        }
    }

    // Mark search tables as out of date
    if (ok) {
        ok = this->setSearchUpdate(1);
    }

    return ok;
}

bool Database::updateSong(Metadata::Song m) {
    // First check we have write permission
    if (this->db->connectionType() != SQLite::Connection::ReadWrite) {
        this->setErrorMsg("[updateSong] Can't update song as the database is unwritable");
        return false;
    }

    // Add artist and album (will do nothing if they already exist)
    bool ok = this->addArtist(m.artist);
    ok = keepFalse(ok, this->addAlbum(m.album));
    if (!ok) {
        return false;
    }

    // Now update relevant fields
    ok = this->db->prepareQuery("UPDATE Songs SET modified = ?, artist_id = (SELECT id FROM Artists WHERE name = ?), album_id = (SELECT id FROM Albums WHERE name = ?), title = ?, track = ?, disc = ?, duration = ?, plays = ?, favourite = ?, path = ? WHERE id = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, m.modified));
    ok = keepFalse(ok, this->db->bindString(1, m.artist));
    ok = keepFalse(ok, this->db->bindString(2, m.album));
    ok = keepFalse(ok, this->db->bindString(3, m.title));
    ok = keepFalse(ok, this->db->bindInt(4, m.trackNumber));
    ok = keepFalse(ok, this->db->bindInt(5, m.discNumber));
    ok = keepFalse(ok, this->db->bindInt(6, m.duration));
    ok = keepFalse(ok, this->db->bindInt(7, m.plays));
    ok = keepFalse(ok, this->db->bindBool(8, m.favourite));
    ok = keepFalse(ok, this->db->bindString(9, m.path));
    ok = keepFalse(ok, this->db->bindInt(10, m.ID));
    if (!ok) {
        this->setErrorMsg("[updateSong] An error occurred while preparing the statement");
        return false;
    }

    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[updateSong] An error occurred while adding the entry");
    } else {
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("[DB] [updateSong] '" + m.title + "' was updated");
        }
    }

    // Mark search tables as out of date
    if (ok) {
        ok = this->setSearchUpdate(1);
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

    // Mark search tables as out of date
    if (ok) {
        ok = this->setSearchUpdate(1);
    }

    return ok;
}

std::vector<Metadata::Song> Database::getAllSongMetadata() {
    std::vector<Metadata::Song> v;
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getAllSongInfo] No open connection");
        return v;
    }

    // Create a Metadata::Song for each entry
    bool ok = this->db->prepareQuery("SELECT Songs.ID, Songs.title, Artists.name, Albums.name, Songs.track, Songs.disc, Songs.duration, Songs.plays, Songs.favourite, Songs.path, Songs.modified FROM Songs JOIN Albums ON Albums.id = Songs.album_id JOIN Artists ON Artists.id = Songs.artist_id;");
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getAllSongInfo] Unable to query for all songs");
        return v;
    }
    while (ok) {
        Metadata::Song m;
        int tmp;
        ok = this->db->getInt(0, m.ID);
        ok = keepFalse(ok, this->db->getString(1, m.title));
        ok = keepFalse(ok, this->db->getString(2, m.artist));
        ok = keepFalse(ok, this->db->getString(3, m.album));
        ok = keepFalse(ok, this->db->getInt(4, tmp));
        m.trackNumber = tmp;
        ok = keepFalse(ok, this->db->getInt(5, tmp));
        m.discNumber = tmp;
        ok = keepFalse(ok, this->db->getInt(6, tmp));
        m.duration = tmp;
        ok = keepFalse(ok, this->db->getInt(7, tmp));
        m.plays = tmp;
        ok = keepFalse(ok, this->db->getBool(8, m.favourite));
        ok = keepFalse(ok, this->db->getString(9, m.path));
        ok = keepFalse(ok, this->db->getInt(10, tmp));
        m.modified = tmp;

        if (ok) {
            v.push_back(m);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    v.shrink_to_fit();
    return v;
}

std::vector<Metadata::Song> Database::getSongMetadataForAlbum(AlbumID id) {
    std::vector<Metadata::Song> v;
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getSongMetadataForAlbum] No open connection");
        return v;
    }

    // Create a Metadata::Song for each entry given the album
    bool ok = this->db->prepareQuery("SELECT Songs.ID, Songs.title, Artists.name, Albums.name, Songs.track, Songs.disc, Songs.duration, Songs.plays, Songs.favourite, Songs.path, Songs.modified FROM Songs JOIN Albums ON Albums.id = Songs.album_id JOIN Artists ON Artists.id = Songs.artist_id WHERE Songs.album_id = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getSongMetadataForAlbum] Unable to query for matching songs");
        return v;
    }
    while (ok) {
        Metadata::Song m;
        int tmp;
        ok = this->db->getInt(0, m.ID);
        ok = keepFalse(ok, this->db->getString(1, m.title));
        ok = keepFalse(ok, this->db->getString(2, m.artist));
        ok = keepFalse(ok, this->db->getString(3, m.album));
        ok = keepFalse(ok, this->db->getInt(4, tmp));
        m.trackNumber = tmp;
        ok = keepFalse(ok, this->db->getInt(5, tmp));
        m.discNumber = tmp;
        ok = keepFalse(ok, this->db->getInt(6, tmp));
        m.duration = tmp;
        ok = keepFalse(ok, this->db->getInt(7, tmp));
        m.plays = tmp;
        ok = keepFalse(ok, this->db->getBool(8, m.favourite));
        ok = keepFalse(ok, this->db->getString(9, m.path));
        ok = keepFalse(ok, this->db->getInt(10, tmp));
        m.modified = tmp;

        if (ok) {
            v.push_back(m);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    v.shrink_to_fit();
    return v;
}

std::vector<Metadata::Song> Database::getSongMetadataForArtist(ArtistID id) {
    std::vector<Metadata::Song> v;
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getSongMetadataForArtist] No open connection");
        return v;
    }

    // Create a Metadata::Song for each entry given the artist
    bool ok = this->db->prepareQuery("SELECT Songs.ID, Songs.title, Artists.name, Albums.name, Songs.track, Songs.disc, Songs.duration, Songs.plays, Songs.favourite, Songs.path, Songs.modified FROM Songs JOIN Albums ON Albums.id = Songs.album_id JOIN Artists ON Artists.id = Songs.artist_id WHERE Songs.artist_id = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getSongMetadataForArtist] Unable to query for matching songs");
        return v;
    }
    while (ok) {
        Metadata::Song m;
        int tmp;
        ok = this->db->getInt(0, m.ID);
        ok = keepFalse(ok, this->db->getString(1, m.title));
        ok = keepFalse(ok, this->db->getString(2, m.artist));
        ok = keepFalse(ok, this->db->getString(3, m.album));
        ok = keepFalse(ok, this->db->getInt(4, tmp));
        m.trackNumber = tmp;
        ok = keepFalse(ok, this->db->getInt(5, tmp));
        m.discNumber = tmp;
        ok = keepFalse(ok, this->db->getInt(6, tmp));
        m.duration = tmp;
        ok = keepFalse(ok, this->db->getInt(7, tmp));
        m.plays = tmp;
        ok = keepFalse(ok, this->db->getBool(8, m.favourite));
        ok = keepFalse(ok, this->db->getString(9, m.path));
        ok = keepFalse(ok, this->db->getInt(10, tmp));
        m.modified = tmp;

        if (ok) {
            v.push_back(m);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    v.shrink_to_fit();
    return v;
}

Metadata::Song Database::getSongMetadataForID(SongID id) {
    Metadata::Song m;
    m.ID = -1;
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getSongInfoForID] No open connection");
        return m;
    }

    // Query for song info
    bool ok = this->db->prepareQuery("SELECT Songs.ID, Songs.title, Artists.name, Albums.name, Songs.track, Songs.disc, Songs.duration, Songs.plays, Songs.favourite, Songs.path, Songs.modified FROM Songs JOIN Albums ON Albums.id = Songs.album_id JOIN Artists ON Artists.id = Songs.artist_id WHERE Songs.ID = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getSongInfoForID] An error occurred querying for info");
        return m;
    }
    int tmp;
    ok = this->db->getInt(0, m.ID);
    ok = keepFalse(ok, this->db->getString(1, m.title));
    ok = keepFalse(ok, this->db->getString(2, m.artist));
    ok = keepFalse(ok, this->db->getString(3, m.album));
    ok = keepFalse(ok, this->db->getInt(4, tmp));
    m.trackNumber = tmp;
    ok = keepFalse(ok, this->db->getInt(5, tmp));
    m.discNumber = tmp;
    ok = keepFalse(ok, this->db->getInt(6, tmp));
    m.duration = tmp;
    ok = keepFalse(ok, this->db->getInt(7, tmp));
    m.plays = tmp;
    ok = keepFalse(ok, this->db->getBool(8, m.favourite));
    ok = keepFalse(ok, this->db->getString(9, m.path));
    ok = keepFalse(ok, this->db->getInt(10, tmp));
    m.modified = tmp;

    if (!ok) {
        this->setErrorMsg("[getSongInfoForID] An error occurred reading from the query results");
        m.ID = -1;
    }

    return m;
}

// ===== Search Queries ===== //
bool Database::needsSearchUpdate() {
    // Check if we have read permission
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[needsSearchUpdate] Database has not been opened");
        return false;
    }

    // Return true if search_update does not equal 0
    int val;
    bool ok = this->db->prepareAndExecuteQuery("SELECT value FROM Variables WHERE name = 'search_update';");
    ok = keepFalse(ok, this->db->getInt(0, val));
    if (!ok) {
        this->setErrorMsg("[needsSearchUpdate] Unable to query update variable");
        return false;
    }
    return (val != 0);
}

bool Database::prepareSearch() {
    // First check we have write permission
    if (this->db->connectionType() != SQLite::Connection::ReadWrite) {
        this->setErrorMsg("[prepareSearch] Can't perform indexing as the database is unwritable");
        return false;
    }

    // Update fts tables
    bool ok = this->db->prepareAndExecuteQuery("DELETE FROM FtsSongs;");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Unable to empty FtsSongs");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("INSERT INTO FtsSongs SELECT title FROM Songs;");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Failed to populate FtsSongs");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("DELETE FROM FtsArtists;");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Unable to empty FtsArtists");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("INSERT INTO FtsArtists SELECT name FROM Artists;");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Failed to populate FtsArtists");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("DELETE FROM FtsAlbums;");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Unable to empty FtsAlbums");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("INSERT INTO FtsAlbums SELECT name FROM Albums;");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Failed to populate FtsAlbums");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("DELETE FROM FtsPlaylists;");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Unable to empty FtsPlaylists");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("INSERT INTO FtsPlaylists SELECT name FROM Playlists;");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Failed to populate FtsPlaylists");
        return false;
    }

    // Update fts4aux tables (used to populate spellfix tables)
    this->db->prepareAndExecuteQuery("DROP TABLE IF EXISTS FtsAuxSongs;");
    ok = this->db->prepareAndExecuteQuery("CREATE VIRTUAL TABLE FtsAuxSongs USING fts4aux(FtsSongs);");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Failed to create FtsAuxSongs table");
        return false;
    }
    this->db->prepareAndExecuteQuery("DROP TABLE IF EXISTS FtsAuxArtists;");
    ok = this->db->prepareAndExecuteQuery("CREATE VIRTUAL TABLE FtsAuxArtists USING fts4aux(FtsArtists);");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Failed to create FtsAuxArtists table");
        return false;
    }
    this->db->prepareAndExecuteQuery("DROP TABLE IF EXISTS FtsAuxAlbums;");
    ok = this->db->prepareAndExecuteQuery("CREATE VIRTUAL TABLE FtsAuxAlbums USING fts4aux(FtsAlbums);");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Failed to create FtsAuxAlbums table");
        return false;
    }
    this->db->prepareAndExecuteQuery("DROP TABLE IF EXISTS FtsAuxPlaylists;");
    ok = this->db->prepareAndExecuteQuery("CREATE VIRTUAL TABLE FtsAuxPlaylists USING fts4aux(FtsPlaylists);");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Failed to create FtsAuxPlaylists table");
        return false;
    }

    // Now populate spellfix tables
    ok = this->db->prepareAndExecuteQuery("DELETE FROM SpellfixSongs;");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Unable to empty SpellfixSongs");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("INSERT INTO SpellfixSongs (word, rank) SELECT term, documents FROM FtsAuxSongs WHERE col='*';");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Failed to populate SpellfixSongs with terms");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("DELETE FROM SpellfixArtists;");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Unable to empty SpellfixArtists");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("INSERT INTO SpellfixArtists (word, rank) SELECT term, documents FROM FtsAuxArtists WHERE col='*';");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Failed to populate SpellfixArtists with terms");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("DELETE FROM SpellfixAlbums;");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Unable to empty SpellfixAlbums");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("INSERT INTO SpellfixAlbums (word, rank) SELECT term, documents FROM FtsAuxAlbums WHERE col='*';");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Failed to populate SpellfixAlbums with terms");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("DELETE FROM SpellfixPlaylists;");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Unable to empty SpellfixPlaylists");
        return false;
    }
    ok = this->db->prepareAndExecuteQuery("INSERT INTO SpellfixPlaylists (word, rank) SELECT term, documents FROM FtsAuxPlaylists WHERE col='*';");
    if (!ok) {
        this->setErrorMsg("[prepareSearch] Failed to populate SpellfixPlaylists with terms");
        return false;
    }

    // Update variable to indicate no update is needed
    ok = this->setSearchUpdate(0);
    if (ok) {
        Log::writeSuccess("[DB] [prepareSearch] Search tables updated successfully!");
    }
    return ok;
}

std::vector<Metadata::Album> Database::searchAlbums(std::string str) {
    std::vector<Metadata::Album> v;

    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[searchAlbums] No open connection");
        return v;
    }

    // Split string into words and correct any spelling mistakes
    if (!this->spellfixString("SpellfixAlbums", str)) {
        return v;
    }

    // Perform search
    bool ok = this->db->prepareQuery("SELECT id, name FROM Albums WHERE name IN (SELECT * FROM FtsAlbums WHERE FtsAlbums MATCH ?);");
    ok = keepFalse(ok, this->db->bindString(0, str));
    if (!ok) {
        this->setErrorMsg("[searchAlbums] Unable to prepare search query");
        return v;
    }
    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[searchAlbums] Unable to search database");
        return v;
    }

    // Collate results
    while (ok) {
        Metadata::Album m;
        ok = this->db->getInt(0, m.ID);
        ok = keepFalse(ok, this->db->getString(1, m.name));

        if (ok) {
            v.push_back(m);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    return v;
}

std::vector<Metadata::Artist> Database::searchArtists(std::string str) {
    std::vector<Metadata::Artist> v;

    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[searchArtists] No open connection");
        return v;
    }

    // Split string into words and correct any spelling mistakes
    if (!this->spellfixString("SpellfixArtists", str)) {
        return v;
    }

    // Perform search
    bool ok = this->db->prepareQuery("SELECT id, name, tadb_id, image_path FROM Artists WHERE name IN (SELECT * FROM FtsArtists WHERE FtsArtists MATCH ?);");
    ok = keepFalse(ok, this->db->bindString(0, str));
    if (!ok) {
        this->setErrorMsg("[searchArtists] Unable to prepare search query");
        return v;
    }
    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[searchArtists] Unable to search database");
        return v;
    }

    // Collate results
    while (ok) {
        Metadata::Artist m;
        ok = this->db->getInt(0, m.ID);
        ok = keepFalse(ok, this->db->getString(1, m.name));
        ok = keepFalse(ok, this->db->getInt(2, m.tadbID));
        ok = keepFalse(ok, this->db->getString(3, m.imagePath));
        m.songCount = 0;    // These should probably be implemented here?
        m.albumCount = 0;

        if (ok) {
            v.push_back(m);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    return v;
}

std::vector<Metadata::Playlist> Database::searchPlaylists(std::string str) {
    std::vector<Metadata::Playlist> v;

    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[searchPlaylists] No open connection");
        return v;
    }

    // Split string into words and correct any spelling mistakes
    if (!this->spellfixString("SpellfixPlaylists", str)) {
        return v;
    }

    // Perform search
    bool ok = this->db->prepareQuery("SELECT id, name, description FROM Playlists WHERE name IN (SELECT * FROM FtsPlaylists WHERE FtsPlaylists MATCH ?);");
    ok = keepFalse(ok, this->db->bindString(0, str));
    if (!ok) {
        this->setErrorMsg("[searchPlaylists] Unable to prepare search query");
        return v;
    }
    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[searchPlaylists] Unable to search database");
        return v;
    }

    // Collate results
    while (ok) {
        Metadata::Playlist m;
        ok = this->db->getInt(0, m.ID);
        ok = keepFalse(ok, this->db->getString(1, m.name));
        ok = keepFalse(ok, this->db->getString(2, m.description));

        if (ok) {
            v.push_back(m);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    return v;
}

std::vector<Metadata::Song> Database::searchSongs(std::string str) {
    std::vector<Metadata::Song> v;

    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[searchSongs] No open connection");
        return v;
    }

    // Split string into words and correct any spelling mistakes
    if (!this->spellfixString("SpellfixSongs", str)) {
        return v;
    }

    // Perform search
    bool ok = this->db->prepareQuery("SELECT Songs.id, Songs.title, Artists.name, Albums.name, Songs.track, Songs.disc, Songs.duration, Songs.plays, Songs.favourite, Songs.path, Songs.modified FROM Songs JOIN Albums ON Albums.id = Songs.album_id JOIN Artists ON Artists.id = Songs.artist_id WHERE Songs.title IN (SELECT * FROM FtsSongs WHERE FtsSongs MATCH ?);");
    ok = keepFalse(ok, this->db->bindString(0, str));
    if (!ok) {
        this->setErrorMsg("[searchSongs] Unable to prepare search query");
        return v;
    }
    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[searchSongs] Unable to search database");
        return v;
    }

    // Collate results
    while (ok) {
        Metadata::Song m;
        int tmp;
        ok = this->db->getInt(0, m.ID);
        ok = keepFalse(ok, this->db->getString(1, m.title));
        ok = keepFalse(ok, this->db->getString(2, m.artist));
        ok = keepFalse(ok, this->db->getString(3, m.album));
        ok = keepFalse(ok, this->db->getInt(4, tmp));
        m.trackNumber = tmp;
        ok = keepFalse(ok, this->db->getInt(5, tmp));
        m.discNumber = tmp;
        ok = keepFalse(ok, this->db->getInt(6, tmp));
        m.duration = tmp;
        ok = keepFalse(ok, this->db->getInt(7, tmp));
        m.plays = tmp;
        ok = keepFalse(ok, this->db->getBool(8, m.favourite));
        ok = keepFalse(ok, this->db->getString(9, m.path));
        ok = keepFalse(ok, this->db->getInt(10, tmp));
        m.modified = tmp;

        if (ok) {
            v.push_back(m);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    return v;
}

// ===== Misc. Queries ===== //
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

ArtistID Database::getArtistIDForName(const std::string & name) {
    int aID = -1;

    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getArtistIDForName] No open connection");
        return aID;
    }

    // Get ArtistID first
    bool ok = this->db->prepareQuery("SELECT id FROM Artists WHERE name = ?;");
    ok = keepFalse(ok, this->db->bindString(0, name));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getArtistIDForName] An error occurred querying for info");
        return aID;
    }
    if (!this->db->getInt(0, aID)) {
        this->setErrorMsg("[getArtistIDForName] An error occurred reading the id");
    }
    return aID;
}

ArtistID Database::getArtistIDForSong(SongID id) {
    int aID = -1;

    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getArtistIDForSong] No open connection");
        return aID;
    }

    // Get ArtistID first
    bool ok = this->db->prepareQuery("SELECT artist_id FROM Songs WHERE id = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getArtistIDForSong] An error occurred querying for info");
        return aID;
    }
    if (!this->db->getInt(0, aID)) {
        this->setErrorMsg("[getArtistIDForSong] An error occurred reading the id");
    }
    return aID;
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

// ===== Destructor ===== //
Database::~Database() {
    this->close();
}