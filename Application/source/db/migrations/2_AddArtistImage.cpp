#include "db/migrations/2_AddArtistImage.hpp"

namespace Migration {
    std::string migrateTo2(SQLite * db) {
        // Add MusicBrainz ID column (starts with empty string)
        bool ok = db->prepareAndExecuteQuery("ALTER TABLE Artists ADD COLUMN tadb_id INTEGER NOT NULL DEFAULT -1;");
        if (!ok) {
            return "Unable to add tadb_id column to Artists";
        }

        // Add image path column (also starts empty)
        ok = db->prepareAndExecuteQuery("ALTER TABLE Artists ADD COLUMN image_path TEXT NOT NULL DEFAULT '';");
        if (!ok) {
            return "Unable to add image_path column to Artists";
        }

        // Bump up version number (only done if everything passes)
        ok = db->prepareAndExecuteQuery("UPDATE Variables SET value = 2 WHERE name = 'version';");
        if (!ok) {
            return "Unable to set version to 2";
        }

        return "";
    }
};