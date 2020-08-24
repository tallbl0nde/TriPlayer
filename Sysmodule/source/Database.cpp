#include "Database.hpp"
#include "Log.hpp"

// Location of the database file
#define DB_PATH "/switch/TriPlayer/data.sqlite3"
// Version of the database (database begins with zero from 'template', so this started at 1)
#define DB_VERSION 6

// Custom boolean 'operator' which instead of 'keeping' true, will 'keep' false
bool keepFalse(const bool & a, const bool & b) {
    return !(!a || !b);
}

Database::Database() {
    // Create the database object
    this->db = new SQLite(DB_PATH);
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
        Log::writeError("[DB] Unable to get the database's version");
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
        Log::writeError("[DB] Version mismatch! Please check that the sysmodule is up to date! (Database: " + std::to_string(version) + ", Supports: " + std::to_string(DB_VERSION) + ")");
        return false;
    }

    return true;
}

// ============================ //
//       PUBLIC FUNCTIONS       //
// ============================ //
bool Database::openReadWrite() {
    if (!this->db->openConnection(SQLite::Connection::ReadWrite)) {
        return false;
    }

    // Close if version is not supported
    if (!this->matchingVersion()) {
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

std::string Database::getPathForID(SongID id) {
    // Check we can read
    if (this->db->connectionType() == SQLite::Connection::None) {
        Log::writeError("[DB] [getPathForID] No open connection");
        return "";
    }

    // Query path
    std::string path;
    bool ok = this->db->prepareQuery("SELECT path FROM Songs WHERE id = ?;");
    ok = keepFalse(ok, this->db->bindInt(0, id));
    ok = keepFalse(ok, this->db->executeQuery());
    ok = keepFalse(ok, this->db->getString(0, path));
    if (!ok) {
        Log::writeError("[DB] [getPathForID] An error occurred querying the path");
        return "";
    }

    path.shrink_to_fit();
    return path;
}

Database::~Database() {
    this->close();
}