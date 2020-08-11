#include "db/Database.hpp"
#include "db/migrations/Migration.hpp"
#include "db/okapi_bm25.h"
#include "db/Spellfix.h"
#include "Log.hpp"
#include "utils/FS.hpp"
#include "utils/Utils.hpp"

// Location of backup file
#define BACKUP_PATH "/switch/TriPlayer/data_old.sqlite3"
// Location of the database file
#define DB_PATH "/switch/TriPlayer/data.sqlite3"
// Version of the database (database begins with zero from 'template', so this started at 1)
#define DB_VERSION 5
// Maximum number of phrases to search for using spellfixed strings
#define SEARCH_PHRASES 6
// Maximum 'spellfix score' allowed when fixing a word
#define SPELLFIX_SCORE 150
// Location of template file
#define TEMPLATE_DB_PATH "romfs:/db/template.sqlite3"

// Custom boolean 'operator' which instead of 'keeping' true, will 'keep' false
bool keepFalse(const bool & a, const bool & b) {
    return !(!a || !b);
}

// Helper function which takes a vector of vectors containing spellfixed words,
// which forms all possible phrases and calculates the score of each phrase
// It uses recursion to search 'down' the next vector and concatenate words
std::vector<SpellfixString> combineWords(std::vector< std::vector<SpellfixString> > & words, size_t v) {
    // Vector to return, will be populated with combinatins
    std::vector<SpellfixString> strings;

    // Base case - stop if going past end of vector
    if (v >= words.size()) {
        return strings;

    // Base case - if we're up to the last vector, return the vector
    } else if (v == words.size() - 1) {
        return words[v];
    }

    // Edge case - skip over empty vectors
    if (words[v].empty()) {
        return combineWords(words, v+1);
    }

    // Form combinations by recursively concatenating entire vectors with
    // the currently chosen word
    for (size_t i = 0; i < words[v].size(); i++) {
        std::vector<SpellfixString> subset = combineWords(words, v+1);
        for (size_t j = 0; j < subset.size(); j++) {
            // Combine both strings and scores
            SpellfixString tmp;
            tmp.string = words[v][i].string + " " + subset[j].string;
            tmp.score = words[v][i].score + subset[j].score;
            strings.push_back(tmp);
        }
    }

    return strings;
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
    // Load the okapi_bm25 extension
    sqlite3_auto_extension((void (*)(void))sqlite3_okapi_bm25_init);

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

            case 3:
                err = Migration::migrateTo4(this->db);
                if (!err.empty()) {
                    err = "Migration 4: " + err;
                    break;
                }
                Log::writeSuccess("[DB] Migrated to version 4");

            case 4:
                err = Migration::migrateTo5(this->db);
                if (!err.empty()) {
                    err = "Migration 5: " + err;
                    break;
                }
                Log::writeSuccess("[DB] Migrated to version 5");
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

std::vector<SpellfixString> Database::getSearchPhrases(const std::string & table, std::string & str) {
    // Split string into words
    std::vector<std::string> words = Utils::splitIntoWords(str);
    str = "";

    // Get word suggestions for each word and store associated score
    std::vector< std::vector<SpellfixString> > suggestions;
    for (size_t i = 0; i < words.size(); i++) {
        // Use string concatenation here as you can't bind a table name (I know it's not ideal but the names are hard coded at least)
        bool ok = this->db->prepareQuery("SELECT word, score FROM " + table + " WHERE word MATCH ? AND SCORE < ?;");
        ok = keepFalse(ok, this->db->bindString(0, words[i]));
        ok = keepFalse(ok, this->db->bindInt(1, SPELLFIX_SCORE));
        ok = keepFalse(ok, this->db->executeQuery());
        if (!ok) {
            this->setErrorMsg("Error performing spell check");
        }

        // Push returned words onto vector
        std::vector<SpellfixString> fixed;
        while (ok && this->db->hasRow()) {
            SpellfixString word;
            ok = keepFalse(ok, this->db->getString(0, word.string));
            ok = keepFalse(ok, this->db->getInt(1, word.score));

            if (ok) {
                fixed.push_back(word);
            }
            ok = keepFalse(ok, this->db->nextRow());
        }
        suggestions.push_back(fixed);
    }

    // Now form phrases using all possible combinations of words (in order)
    std::vector<SpellfixString> phrases = combineWords(suggestions, 0);

    // Sort so that phrases with lower scores are used first
    std::sort(phrases.begin(), phrases.end(), [](const SpellfixString & lhs, const SpellfixString & rhs) {
        return lhs.score < rhs.score;
    });

    // Limit number of phrases
    if (phrases.size() > SEARCH_PHRASES) {
        phrases = std::vector<SpellfixString>(phrases.begin(), phrases.begin() + SEARCH_PHRASES);
    }
    return phrases;
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
bool Database::updateAlbum(Metadata::Album m) {
    // First check we have write permission
    if (this->db->connectionType() != SQLite::Connection::ReadWrite) {
        this->setErrorMsg("[updateAlbum] Can't update song as the database is unwritable");
        return false;
    }

    // Now update relevant fields
    bool ok = this->db->prepareQuery("UPDATE Albums SET name = ?, tadb_id = ?, image_path = ? WHERE id = ?;");
    ok = keepFalse(ok, this->db->bindString(0, m.name));
    ok = keepFalse(ok, this->db->bindInt(1, m.tadbID));
    ok = keepFalse(ok, this->db->bindString(2, m.imagePath));
    ok = keepFalse(ok, this->db->bindInt(3, m.ID));
    if (!ok) {
        this->setErrorMsg("[updateAlbum] An error occurred while preparing the statement");
        return false;
    }

    // We don't want to ignore constraints for this query
    this->db->ignoreConstraints(false);
    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[updateAlbum] An error occurred while updating the entry");
    } else {
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("[DB] [updateAlbum] '" + m.name + "' was updated");
        }
    }
    this->db->ignoreConstraints(true);

    // Mark search tables as out of date
    if (ok) {
        ok = this->setSearchUpdate(1);
    }

    return ok;
}

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
    while (ok  && this->db->hasRow()) {
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
    while (ok && this->db->hasRow()) {
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
    while (ok && this->db->hasRow()) {
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
    while (ok && this->db->hasRow()) {
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

// ===== Playlist Metadata ===== //
bool Database::addPlaylist(Metadata::Playlist m) {
    // First check we have write permission
    if (this->db->connectionType() != SQLite::Connection::ReadWrite) {
        this->setErrorMsg("[addPlaylist] Can't add a playlist as the database is unwritable");
        return false;
    }

    // Prepare query
    bool ok = this->db->prepareQuery("INSERT INTO Playlists (name, description, image_path) VALUES (?, ?, ?);");
    ok = keepFalse(ok, this->db->bindString(0, m.name));
    ok = keepFalse(ok, this->db->bindString(1, m.description));
    ok = keepFalse(ok, this->db->bindString(2, m.imagePath));
    if (!ok) {
        this->setErrorMsg("[addPlaylist] An error occurred while preparing the statement");
        return false;
    }

    // Perform the query
    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[addPlaylist] An error occurred while adding the entry");
    } else {
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("[DB] [addPlaylist] '" + m.name + "' added to the database");
        }
    }

    // Mark search tables as out of date
    if (ok) {
        ok = this->setSearchUpdate(1);
    }

    return ok;
}

bool Database::updatePlaylist(Metadata::Playlist m) {
    // First check we have write permission
    if (this->db->connectionType() != SQLite::Connection::ReadWrite) {
        this->setErrorMsg("[updatePlaylist] Can't add a playlist as the database is unwritable");
        return false;
    }

    // Prepare query
    bool ok = this->db->prepareQuery("UPDATE Playlists SET name = ?, description = ?, image_path = ? WHERE id = ?;");
    ok = keepFalse(ok, this->db->bindString(0, m.name));
    ok = keepFalse(ok, this->db->bindString(1, m.description));
    ok = keepFalse(ok, this->db->bindString(2, m.imagePath));
    ok = keepFalse(ok, this->db->bindInt(3, m.ID));
    if (!ok) {
        this->setErrorMsg("[updatePlaylist] An error occurred while preparing the statement");
    }

    // Perform the query
    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[updatePlaylist] An error occurred while updating the entry");
    } else {
        if (Log::loggingLevel() == Log::Level::Info) {
            Log::writeInfo("[DB] [updatePlaylist] '" + m.name + "' was updated");
        }
    }

    // Mark search tables as out of date
    if (ok) {
        ok = this->setSearchUpdate(1);
    }

    return ok;
}

bool Database::removePlaylist(PlaylistID id) {
    // First check we have write permission
    if (this->db->connectionType() != SQLite::Connection::ReadWrite) {
        this->setErrorMsg("[removePlaylist] Can't add a playlist as the database is unwritable");
        return false;
    }

    // Prepare and execute query to remove playlist (songs are deleted due to cascade)
    bool ok = this->db->prepareQuery("DELETE FROM Playlists WHERE id = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    if (!ok) {
        this->setErrorMsg("[removePlaylist] An error occurred preparing the query");
        return false;
    }
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[removePlaylist] An error occurred removing the playlist");
    }

    // Mark search tables as out of date
    if (ok) {
        ok = this->setSearchUpdate(1);
    }

    return ok;
}

std::vector<Metadata::Playlist> Database::getAllPlaylistMetadata() {
    std::vector<Metadata::Playlist> v;
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getAllPlaylistMetadata] No open connection");
        return v;
    }

    // Create a Metadata::Playlist for each entry
    bool ok = this->db->prepareAndExecuteQuery("SELECT id, name, description, image_path, COUNT(PlaylistSongs.song_id) FROM Playlists LEFT JOIN PlaylistSongs ON playlist_id = Playlists.id GROUP BY Playlists.id ORDER BY name;");
    if (!ok) {
        this->setErrorMsg("[getAllPlaylistMetadata] Unable to query for all playlists");
        return v;
    }
    while (ok && this->db->hasRow()) {
        Metadata::Playlist m;
        int tmp;
        ok = this->db->getInt(0, m.ID);
        ok = keepFalse(ok, this->db->getString(1, m.name));
        ok = keepFalse(ok, this->db->getString(2, m.description));
        ok = keepFalse(ok, this->db->getString(3, m.imagePath));
        ok = keepFalse(ok, this->db->getInt(4, tmp));
        m.songCount = tmp;

        if (ok) {
            v.push_back(m);
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    v.shrink_to_fit();
    return v;
}

Metadata::Playlist Database::getPlaylistMetadataForID(PlaylistID id) {
    Metadata::Playlist m;
    m.ID = -1;
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getPlaylistMetadataForID] No open connection");
        return m;
    }

    // Query for playlist info
    bool ok = this->db->prepareQuery("SELECT id, name, description, image_path, COUNT(PlaylistSongs.song_id) FROM Playlists LEFT JOIN PlaylistSongs ON playlist_id = Playlists.id WHERE id = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getPlaylistMetadataForID] An error occurred querying for info");
        return m;
    }
    int tmp;
    ok = this->db->getInt(0, m.ID);
    ok = keepFalse(ok, this->db->getString(1, m.name));
    ok = keepFalse(ok, this->db->getString(2, m.description));
    ok = keepFalse(ok, this->db->getString(3, m.imagePath));
    ok = keepFalse(ok, this->db->getInt(4, tmp));
    m.songCount = tmp;

    if (!ok) {
        this->setErrorMsg("[getPlaylistMetadataForID] An error occurred reading from the query results");
        m.ID = -1;
    }

    return m;
}


std::vector<Metadata::PlaylistSong> Database::getSongMetadataForPlaylist(PlaylistID id) {
    std::vector<Metadata::PlaylistSong> v;
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getSongMetadataForPlaylist] No open connection");
        return v;
    }

    // Create a Metadata::Song for each entry given the playlist
    bool ok = this->db->prepareQuery("SELECT Songs.ID, Songs.title, Artists.name, Albums.name, Songs.track, Songs.disc, Songs.duration, Songs.plays, Songs.favourite, Songs.path, Songs.modified, PlaylistSongs.rowid FROM PlaylistSongs JOIN Songs ON Songs.id = PlaylistSongs.song_id JOIN Albums ON Albums.id = Songs.album_id JOIN Artists ON Artists.id = Songs.artist_id WHERE PlaylistSongs.playlist_id = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getSongMetadataForPlaylist] Unable to query for matching songs");
        return v;
    }
    while (ok && this->db->hasRow()) {
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
        ok = keepFalse(ok, this->db->getInt(11, tmp));

        // Push back PlaylistSong struct if all successful
        if (ok) {
            v.push_back(Metadata::PlaylistSong{tmp, m});
        }
        ok = keepFalse(ok, this->db->nextRow());
    }

    v.shrink_to_fit();
    return v;
}

bool Database::addSongToPlaylist(PlaylistID pl, SongID s) {
    // First check we have write permission
    if (this->db->connectionType() != SQLite::Connection::ReadWrite) {
        this->setErrorMsg("[addSongToPlaylist] Can't add song as the database is unwritable");
        return false;
    }

    // Prepare query
    bool ok = this->db->prepareQuery("INSERT INTO PlaylistSongs (playlist_id, song_id) VALUES (?, ?);");
    ok = keepFalse(ok, this->db->bindInt(0, pl));
    ok = keepFalse(ok, this->db->bindInt(1, s));
    if (!ok) {
        this->setErrorMsg("[addSongToPlaylist] An error occurred preparing the query");
        return false;
    }

    // Perform query
    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[addSongToPlaylist] An error occurred adding the song");
    }

    return ok;
}

bool Database::removeSongFromPlaylist(PlaylistSongID rowid) {
    // First check we have write permission
    if (this->db->connectionType() != SQLite::Connection::ReadWrite) {
        this->setErrorMsg("[removeSongFromPlaylist] Can't add song as the database is unwritable");
        return false;
    }

    // Prepare query
    bool ok = this->db->prepareQuery("DELETE FROM PlaylistSongs WHERE rowid = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, rowid));
    if (!ok) {
        this->setErrorMsg("[removeSongFromPlaylist] An error occurred preparing the query");
        return false;
    }

    // Perform query
    ok = this->db->executeQuery();
    if (!ok) {
        this->setErrorMsg("[removeSongFromPlaylist] An error occurred removing the song");
    }

    return ok;
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
    while (ok && this->db->hasRow()) {
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

    // Create a Metadata::Song for each entry given the album (sorted)
    // Note that 0's are treated as 9999's so they are at the end (yes this means it won't always be at the end but no album has 9999 discs or 9999 tracks)
    bool ok = this->db->prepareQuery("SELECT Songs.ID, Songs.title, Artists.name, Albums.name, Songs.track, Songs.disc, Songs.duration, Songs.plays, Songs.favourite, Songs.path, Songs.modified FROM Songs JOIN Albums ON Albums.id = Songs.album_id JOIN Artists ON Artists.id = Songs.artist_id WHERE Songs.album_id = ? ORDER BY CASE disc WHEN 0 THEN 9999 ELSE disc END, CASE track WHEN 0 THEN 9999 ELSE track END, title;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getSongMetadataForAlbum] Unable to query for matching songs");
        return v;
    }
    while (ok && this->db->hasRow()) {
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
    while (ok && this->db->hasRow()) {
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
    ok = this->db->prepareAndExecuteQuery("INSERT INTO FtsSongs SELECT title, Artists.name, Albums.name FROM Songs JOIN Artists ON artist_id = Artists.id JOIN Albums ON album_id = Albums.id;");
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

    // Fix any spelling mistakes and get suggested searches based on input
    std::vector<SpellfixString> phrases = this->getSearchPhrases("SpellfixAlbums", str);
    if (phrases.empty()) {
        return v;
    }

    // Iterate over each phrase and store results
    for (size_t i = 0; i < phrases.size(); i++) {
        bool ok = this->db->prepareQuery("SELECT id, name FROM FtsAlbums JOIN Albums ON content = name WHERE FtsAlbums MATCH ? ORDER BY okapi_bm25(matchinfo(FtsAlbums, 'pcxnal'), 0) DESC;");
        std::string tmp = phrases[i].string;
        ok = keepFalse(ok, this->db->bindString(0, tmp));
        ok = keepFalse(ok, this->db->executeQuery());
        if (!ok) {
            this->setErrorMsg("[searchPlaylists] An error occurred searching with the phrase: " + phrases[i].string);
            return v;
        }

        // Iterate over returned rows
        while (ok && this->db->hasRow()) {
            Metadata::Album m;
            ok = this->db->getInt(0, m.ID);
            ok = keepFalse(ok, this->db->getString(1, m.name));

            if (ok) {
                v.push_back(m);
            }
            ok = keepFalse(ok, this->db->nextRow());
        }
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

    // Fix any spelling mistakes and get suggested searches based on input
    std::vector<SpellfixString> phrases = this->getSearchPhrases("SpellfixArtists", str);
    if (phrases.empty()) {
        return v;
    }

    // Iterate over each phrase and store results
    for (size_t i = 0; i < phrases.size(); i++) {
        bool ok = this->db->prepareQuery("SELECT id, name, tadb_id, image_path FROM FtsArtists JOIN Artists ON content = name WHERE FtsArtists MATCH ? ORDER BY okapi_bm25(matchinfo(FtsArtists, 'pcxnal'), 0) DESC;");
        std::string tmp = phrases[i].string;
        ok = keepFalse(ok, this->db->bindString(0, tmp));
        ok = keepFalse(ok, this->db->executeQuery());
        if (!ok) {
            this->setErrorMsg("[searchArtists] An error occurred searching with the phrase: " + phrases[i].string);
            return v;
        }

        // Iterate over returned rows
        while (ok && this->db->hasRow()) {
            Metadata::Artist m;
            ok = this->db->getInt(0, m.ID);
            ok = keepFalse(ok, this->db->getString(1, m.name));
            ok = keepFalse(ok, this->db->getInt(2, m.tadbID));
            ok = keepFalse(ok, this->db->getString(3, m.imagePath));
            m.songCount = 0;    // I haven't been able to figure out how to get these in this query
            m.albumCount = 0;

            if (ok) {
                v.push_back(m);
            }
            ok = keepFalse(ok, this->db->nextRow());
        }
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

    // Fix any spelling mistakes and get suggested searches based on input
    std::vector<SpellfixString> phrases = this->getSearchPhrases("SpellfixPlaylists", str);
    if (phrases.empty()) {
        return v;
    }

    // Iterate over each phrase and store results
    for (size_t i = 0; i < phrases.size(); i++) {
        bool ok = this->db->prepareQuery("SELECT id, name, description FROM FtsPlaylists JOIN Playlists ON content = name WHERE FtsPlaylists MATCH ? ORDER BY okapi_bm25(matchinfo(FtsPlaylists, 'pcxnal'), 0) DESC;");
        std::string tmp = phrases[i].string;
        ok = keepFalse(ok, this->db->bindString(0, tmp));
        ok = keepFalse(ok, this->db->executeQuery());
        if (!ok) {
            this->setErrorMsg("[searchPlaylists] An error occurred searching with the phrase: " + phrases[i].string);
            return v;
        }

        // Iterate over returned rows
        while (ok && this->db->hasRow()) {
            Metadata::Playlist m;
            ok = this->db->getInt(0, m.ID);
            ok = keepFalse(ok, this->db->getString(1, m.name));
            ok = keepFalse(ok, this->db->getString(2, m.description));

            if (ok) {
                v.push_back(m);
            }
            ok = keepFalse(ok, this->db->nextRow());
        }
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

    // Fix any spelling mistakes and get suggested searches based on input
    std::vector<SpellfixString> phrases = this->getSearchPhrases("SpellfixSongs", str);
    if (phrases.empty()) {
        return v;
    }

    // Iterate over each phrase and store results
    for (size_t i = 0; i < phrases.size(); i++) {
        bool ok = this->db->prepareQuery("SELECT Songs.id, Songs.title, Artists.name, Albums.name, Songs.track, Songs.disc, Songs.duration, Songs.plays, Songs.favourite, Songs.path, Songs.modified FROM Songs JOIN FtsSongs ON Songs.title = FtsSongs.title JOIN Artists ON artist_id = Artists.id JOIN Albums ON album_id = Albums.id WHERE FtsSongs MATCH ? ORDER BY okapi_bm25(matchinfo(FtsSongs, 'pcxnal'), 0) DESC;");
        std::string tmp = phrases[i].string;
        ok = keepFalse(ok, this->db->bindString(0, tmp));
        ok = keepFalse(ok, this->db->executeQuery());
        if (!ok) {
            this->setErrorMsg("[searchSongs] An error occurred searching with the phrase: " + phrases[i].string);
            return v;
        }

        // Iterate over returned rows
        while (ok && this->db->hasRow()) {
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
    while (ok && this->db->hasRow()) {
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

AlbumID Database::getAlbumIDForSong(SongID id) {
    int aID = -1;

    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        this->setErrorMsg("[getAlbumIDForSong] No open connection");
        return aID;
    }

    // Get AlbumID first
    bool ok = this->db->prepareQuery("SELECT album_id FROM Songs WHERE id = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (!ok) {
        this->setErrorMsg("[getAlbumIDForSong] An error occurred querying for info");
        return aID;
    }
    if (!this->db->getInt(0, aID)) {
        this->setErrorMsg("[getAlbumIDForSong] An error occurred reading the id");
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