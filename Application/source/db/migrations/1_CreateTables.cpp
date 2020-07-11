#include "1_CreateTables.hpp"

namespace Migration {
    std::string migrateTo1(SQLite * db) {
        // Create Artists table
        bool ok = db->prepareAndExecuteQuery("CREATE TABLE Artists (id INTEGER NOT NULL PRIMARY KEY, name TEXT UNIQUE NOT NULL);");
        if (!ok) {
            return "Unable to create the Artists table";
        }

        // Create Albums table
        ok = db->prepareAndExecuteQuery("CREATE TABLE Albums (id INTEGER NOT NULL PRIMARY KEY, name TEXT UNIQUE NOT NULL);");
        if (!ok) {
            return "Unable to create the Albums table";
        }

        // Create Songs table
        ok = db->prepareAndExecuteQuery("CREATE TABLE Songs (id INTEGER NOT NULL PRIMARY KEY, path TEXT UNIQUE NOT NULL, modified DATETIME NOT NULL, artist_id INT NOT NULL, album_id INT NOT NULL, title TEXT NOT NULL, duration INT NOT NULL, plays INT NOT NULL DEFAULT 0, favourite BOOLEAN NOT NULL DEFAULT 0, FOREIGN KEY (album_id) REFERENCES Albums (id), FOREIGN KEY (artist_id) REFERENCES Artists (id));");
        if (!ok) {
            return "Unable to create the Songs table";
        }

        // Create Playlists table
        ok = db->prepareAndExecuteQuery("CREATE TABLE Playlists (id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL, description TEXT NOT NULL DEFAULT \"\");");
        if (!ok) {
            return "Unable to create the Playlists table";
        }

        // Create PlaylistSongs table
        ok = db->prepareAndExecuteQuery("CREATE TABLE PlaylistSongs (playlist_id INTEGER, song_id INTEGER, FOREIGN KEY (playlist_id) REFERENCES Playlists (id) ON DELETE CASCADE, FOREIGN KEY (song_id) REFERENCES Songs (id) ON DELETE CASCADE);");
        if (!ok) {
            return "Unable to create the Playlists table";
        }

        // Create triggers to delete artists/albums when all their songs are removed
        ok = db->prepareAndExecuteQuery("CREATE TRIGGER deleteArtists AFTER DELETE ON Songs WHEN NOT EXISTS (SELECT 1 FROM Songs WHERE Songs.artist_id = OLD.artist_id) BEGIN DELETE FROM Artists WHERE Artists.id = OLD.artist_id; END;");
        if (!ok) {
            return "Unable to create 'deleteArtists' trigger";
        }
        ok = db->prepareAndExecuteQuery("CREATE TRIGGER deleteAlbums AFTER DELETE ON Songs WHEN NOT EXISTS (SELECT 1 FROM Songs WHERE Songs.album_id = OLD.album_id) BEGIN DELETE FROM Albums WHERE Albums.id = OLD.album_id; END;");
        if (!ok) {
            return "Unable to create 'deleteAlbums' trigger";
        }

        // Create Full Text Search virtual tables for all main tables
        ok = db->prepareAndExecuteQuery("CREATE VIRTUAL TABLE FtsSongs USING fts4");
        if (!ok) {
            return "Failed to create FtsSongs table for FTS";
        }
        ok = db->prepareAndExecuteQuery("CREATE VIRTUAL TABLE FtsArtists USING fts4");
        if (!ok) {
            return "Failed to create FtsArtists table for FTS";
        }
        ok = db->prepareAndExecuteQuery("CREATE VIRTUAL TABLE FtsAlbums USING fts4");
        if (!ok) {
            return "Failed to create FtsAlbums table for FTS";
        }
        ok = db->prepareAndExecuteQuery("CREATE VIRTUAL TABLE FtsPlaylists USING fts4");
        if (!ok) {
            return "Failed to create FtsPlaylists table for FTS";
        }

        // Create 'spellfix' tables (used for roughly fixing spelling mistakes per search type)
        ok = db->prepareAndExecuteQuery("CREATE VIRTUAL TABLE SpellfixSongs USING spellfix1");
        if (!ok) {
            return "Failed to create SpellfixSongs table for spell checking";
        }
        ok = db->prepareAndExecuteQuery("CREATE VIRTUAL TABLE SpellfixArtists USING spellfix1");
        if (!ok) {
            return "Failed to create SpellfixArtists table for spell checking";
        }
        ok = db->prepareAndExecuteQuery("CREATE VIRTUAL TABLE SpellfixAlbums USING spellfix1");
        if (!ok) {
            return "Failed to create SpellfixAlbums table for spell checking";
        }
        ok = db->prepareAndExecuteQuery("CREATE VIRTUAL TABLE SpellfixPlaylists USING spellfix1");
        if (!ok) {
            return "Failed to create SpellfixPlaylists table for spell checking";
        }

        // Create a 'variable' to mark if the search index needs updating
        ok = db->prepareAndExecuteQuery("INSERT INTO Variables (name, value) VALUES ('search_update', 0);");
        if (!ok) {
            return "Unable to create 'search_update' variable";
        }

        // Bump up version number (only done if everything passes)
        ok = db->prepareAndExecuteQuery("UPDATE Variables SET value = 1 WHERE name = 'version';");
        if (!ok) {
            return "Unable to set version to 1";
        }

        return "";
    }
};