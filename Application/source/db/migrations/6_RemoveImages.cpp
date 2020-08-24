#include "db/migrations/6_RemoveImages.hpp"

namespace Migration {
    std::string migrateTo6(SQLite * db) {
        // Add triggers to delete images when a relevant row is deleted
        bool ok = db->prepareAndExecuteQuery("CREATE TRIGGER deleteAlbumImage AFTER DELETE ON Albums WHEN removeImage(OLD.image_path) BEGIN SELECT 0 WHERE 1; END;");
        if (!ok) {
            return "Failed to create 'deleteAlbumImage' trigger";
        }
        ok = db->prepareAndExecuteQuery("CREATE TRIGGER deleteArtistImage AFTER DELETE ON Artists WHEN removeImage(OLD.image_path) BEGIN SELECT 0 WHERE 1; END;");
        if (!ok) {
            return "Failed to create 'deleteArtistImage' trigger";
        }
        ok = db->prepareAndExecuteQuery("CREATE TRIGGER deletePlaylistImage AFTER DELETE ON Playlists WHEN removeImage(OLD.image_path) BEGIN SELECT 0 WHERE 1; END;");
        if (!ok) {
            return "Failed to create 'deletePlaylistImage' trigger";
        }

        // Bump up version number (only done if everything passes)
        ok = db->prepareAndExecuteQuery("UPDATE Variables SET value = 6 WHERE name = 'version';");
        if (!ok) {
            return "Unable to set version to 6";
        }

        return "";
    }
};