#include "Database.hpp"
#include "Log.hpp"
#include "Paths.hpp"
#include "SQLite.hpp"

// Version of the database (database begins with zero from 'template', so this started at 1)
#define DB_VERSION 6

// Custom boolean 'operator' which instead of 'keeping' true, will 'keep' false
bool keepFalse(const bool & a, const bool & b) {
    return !(!a || !b);
}

Database::Database() {
    // Create the database object
    this->db = new SQLite(Path::Common::DatabaseFile);
}

bool Database::getVersion(int & version) {
    bool ok = this->db->prepareQuery("SELECT value FROM Variables WHERE name = 'version';");
    if (ok) {
        ok = this->db->executeQuery();
    }
    if (ok) {
        ok = this->db->getInt(0, version);
    }
    return ok;
}

bool Database::matchingVersion() {
    int version;
    if (!this->getVersion(version)) {
        return false;
    }
    if (version != DB_VERSION) {
        this->close();
        return false;
    }

    return true;
}

bool Database::openReadOnly() {
    if (!this->db->openConnection(SQLite::Connection::ReadOnly)) {
        return false;
    }

    // Close if version is not supported
    if (!this->matchingVersion()) {
        this->close();
        return false;
    }

    return true;
}

void Database::close() {
    this->db->closeConnection();
}

Metadata Database::getMetadataForID(SongID id) {
    Metadata meta;

    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        meta.id = -1;
        return meta;
    }

    // Query for metadata
    std::string path;
    bool ok = this->db->prepareQuery("SELECT Songs.ID, Songs.title, Artists.name, Songs.duration, Albums.image_path FROM Songs JOIN Albums ON Albums.id = Songs.album_id JOIN Artists ON Artists.id = Songs.artist_id WHERE Songs.ID = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    if (ok) {
        ok = keepFalse(ok, this->db->getInt(0, meta.id));
        ok = keepFalse(ok, this->db->getString(1, meta.title));
        ok = keepFalse(ok, this->db->getString(2, meta.artist));
        int tmp;
        ok = keepFalse(ok, this->db->getInt(3, tmp));
        meta.duration = tmp;
        ok = keepFalse(ok, this->db->getString(4, meta.imagePath));
    } else {
        meta.id = -2;
    }

    return meta;
}

Database::~Database() {
    this->close();
}