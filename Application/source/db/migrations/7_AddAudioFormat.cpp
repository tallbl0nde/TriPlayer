#include "db/migrations/7_AddAudioFormat.hpp"

namespace Migration {
    std::string migrateTo7(SQLite * db) {
        // Add audio_format column
        bool ok = db->prepareAndExecuteQuery("ALTER TABLE Songs ADD COLUMN format TEXT NOT NULL DEFAULT '';");
        if (!ok) {
            return "Unable to add format column to Songs";
        }

        // As every entry is an .mp3 at this point, set this value to every row
        ok = db->prepareAndExecuteQuery("UPDATE Songs SET audio_format = 'MP3';");
        if (!ok) {
            return "Unable to initialize all rows' format to 'MP3'";
        }

        // Bump up version number
        ok = db->prepareAndExecuteQuery("UPDATE Variables SET value = 7 WHERE name = 'version';");
        if (!ok) {
            return "Unable to set version to 7";
        }

        return "";
    };
}