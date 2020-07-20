#include "db/migrations/3_AddAlbumMetadata.hpp"

namespace Migration {
    std::string migrateTo3(SQLite * db) {
        // Add TheAudioDB album ID column (starts negative)
        bool ok = db->prepareAndExecuteQuery("ALTER TABLE Albums ADD COLUMN tadb_id INTEGER NOT NULL DEFAULT -1;");
        if (!ok) {
            return "Unable to add tadb_id column to Albums";
        }

        // Add image path column (also starts empty)
        ok = db->prepareAndExecuteQuery("ALTER TABLE Albums ADD COLUMN image_path TEXT NOT NULL DEFAULT '';");
        if (!ok) {
            return "Unable to add image_path column to Albums";
        }

        // Bump up version number (only done if everything passes)
        ok = db->prepareAndExecuteQuery("UPDATE Variables SET value = 3 WHERE name = 'version';");
        if (!ok) {
            return "Unable to set version to 3";
        }

        return "";
    }
};