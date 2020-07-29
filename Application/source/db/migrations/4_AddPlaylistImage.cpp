#include "db/migrations/4_AddPlaylistImage.hpp"

namespace Migration {
    std::string migrateTo4(SQLite * db) {
        // Add image path column (starts empty)
        bool ok = db->prepareAndExecuteQuery("ALTER TABLE Playlists ADD COLUMN image_path TEXT NOT NULL DEFAULT '';");
        if (!ok) {
            return "Unable to add image_path column to Playlists";
        }

        // Bump up version number (only done if everything passes)
        ok = db->prepareAndExecuteQuery("UPDATE Variables SET value = 4 WHERE name = 'version';");
        if (!ok) {
            return "Unable to set version to 4";
        }

        return "";
    }
};