#include "db/migrations/5_UpdateSearch.hpp"

namespace Migration {
    std::string migrateTo5(SQLite * db) {
        // Drop old table
        bool ok = db->prepareAndExecuteQuery("DROP Table FtsSongs;");
        if (!ok) {
            return "Unable to drop FtsSongs";
        }

        // Create new FtsSongs tables which also stores artist and album names
        ok = db->prepareAndExecuteQuery("CREATE VIRTUAL TABLE FtsSongs USING fts4(title, artist, album);");
        if (!ok) {
            return "Unable to create new FtsSongs table";
        }

        // Bump up version number (only done if everything passes)
        ok = db->prepareAndExecuteQuery("UPDATE Variables SET value = 5 WHERE name = 'version';");
        if (!ok) {
            return "Unable to set version to 5";
        }

        return "";
    }
};