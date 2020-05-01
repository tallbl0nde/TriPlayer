#include "Database.hpp"
#include "Log.hpp"

// Database file
#define DB_FILE "/switch/TriPlayer/music.db"

Database::Database() {
    // Open connection
    if (sqlite3_open_v2(DB_FILE, &this->db, SQLITE_OPEN_READONLY, "unix-none") != SQLITE_OK) {
        this->db = nullptr;
        Log::writeError("[DB] Unable to open database R/O");
    }

    // Store journal in memory (otherwise file is unwritable)
    sqlite3_prepare_v2(this->db, "PRAGMA journal_mode=MEMORY;", -1, &this->cmd, NULL);
    if (this->cmd != nullptr) {
        sqlite3_step(this->cmd);
    } else {
        Log::writeError("[DB] Unable to set journal mode to memory");
        sqlite3_close(this->db);
        this->db = nullptr;
    }
    sqlite3_finalize(this->cmd);

    // Note foreign keys aren't required for these operations
    if (this->db != nullptr) {
        Log::writeSuccess("[DB] Prepared for queries");
    }
}

bool Database::ready() {
    return !(this->db == nullptr);
}

std::string Database::getPathForID(SongID id) {
    std::string str = "";

    if (this->db != nullptr) {
        sqlite3_prepare_v2(this->db, "SELECT path FROM Songs WHERE id = ?;", -1, &this->cmd, nullptr);
        if (this->cmd != nullptr) {
            sqlite3_bind_int(this->cmd, 1, id);
            if (sqlite3_step(this->cmd) == SQLITE_ROW) {
                const unsigned char * s = sqlite3_column_text(this->cmd, 0);
                str = std::string((const char *)s);
            }
        }
        sqlite3_finalize(cmd);
    }

    // Log
    if (str.length() == 0) {
        Log::writeError("[DB] Unable to find path for ID: " + std::to_string(id));
    } else {
        Log::writeInfo("[DB] Found path: " + str + " for ID: " + std::to_string(id));
    }

    str.shrink_to_fit();
    return str;
}

Database::~Database() {
    sqlite3_close(this->db);
}