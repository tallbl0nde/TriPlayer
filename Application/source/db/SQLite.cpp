#include "Log.hpp"
#include "SQLite.hpp"

SQLite::SQLite(const std::string & pth) {
    this->path = pth;
    this->path.shrink_to_fit();

    // Ensure all vars are set to default values
    this->connectionType_ = SQLite::Connection::None;
    this->db = nullptr;
    this->errorMsg_ = "";
    this->ignoreConstraints_ = false;
    this->query = nullptr;
    this->queryStatus = SQLite::Query::None;
}

void SQLite::setErrorMsg(const std::string & msg = "") {
    // Write custom message if one is passed
    if (msg.length() > 0) {
        this->errorMsg_ = msg;

    // Write a fatal error if this->db is nullptr
    } else if (this->db == nullptr) {
        this->errorMsg_ = "Fatal error: Database object does not exist!";

    // Form string from SQLite
    } else {
        int errorCode = sqlite3_errcode(this->db);
        const char * errorString = sqlite3_errmsg(this->db);
        this->errorMsg_ = std::string(errorString) + " (" + std::to_string(errorCode) + ")";
    }

    // Also write to application log
    Log::writeError("[SQLITE] " + this->errorMsg_);
}

void SQLite::finalizeQuery() {
    if (this->queryStatus != SQLite::Query::None && this->query != nullptr) {
        sqlite3_finalize(this->query);
    }
    this->query = nullptr;
    this->queryStatus = SQLite::Query::None;
}

bool SQLite::prepare() {
    bool ok = true;

    // Return detailed error codes
    sqlite3_extended_result_codes(this->db, 1);

    // Ensure journal is in memory (DOUBLE CHECK **)
    ok = this->prepareQuery("PRAGMA journal_mode=MEMORY;");
    if (ok) {
        ok = this->executeQuery();
    } else {
        this->setErrorMsg("An error occurred setting the journal mode to MEMORY");
        return false;
    }

    // Ensure foreign keys are used
    ok = this->prepareQuery("PRAGMA foreign_keys=ON;");
    if (ok) {
        ok = this->executeQuery();
    } else {
        this->setErrorMsg("An error occurred enabling foreign keys");
    }

    return ok;
}

std::string SQLite::errorMsg() {
    return this->errorMsg_;
}

void SQLite::ignoreConstraints(bool ign) {
    this->ignoreConstraints_ = ign;
}

SQLite::Connection SQLite::connectionType() {
    return this->connectionType_;
}

void SQLite::closeConnection() {
    // Ensure query is finalized
    this->finalizeQuery();

    // Close database object
    if (this->connectionType_ != SQLite::Connection::None) {
        sqlite3_close(this->db);
        this->db = nullptr;
        Log::writeInfo("[SQLITE] Closed the database");
    }
    this->connectionType_ = SQLite::Connection::None;
}

bool SQLite::openConnection(Connection type) {
    // Check current connection type first
    if (this->connectionType_ != SQLite::Connection::None || type == SQLite::Connection::None) {
        return true;
    }
    this->connectionType_ = type;

    // Open correct type of connection
    int result;
    if (type == SQLite::Connection::ReadOnly) {
        result = sqlite3_open_v2(this->path.c_str(), &this->db, SQLITE_OPEN_READONLY, "unix-none");
        if (result != SQLITE_OK) {
            this->setErrorMsg();
            this->connectionType_ = SQLite::Connection::None;
        } else {
            Log::writeInfo("[SQLITE] Successfully opened read-only connection");
        }

    } else if (type == SQLite::Connection::ReadWrite) {
        result = sqlite3_open_v2(this->path.c_str(), &this->db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, "unix-none");
        if (result != SQLITE_OK) {
            this->setErrorMsg();
            this->connectionType_ = SQLite::Connection::None;
        } else {
            Log::writeInfo("[SQLITE] Successfully opened read-write connection");
        }
    }

    // Set up database
    if (this->connectionType_ != SQLite::Connection::None) {
        return this->prepare();
    }

    return false;
}

bool SQLite::prepareQuery(const std::string & qry) {
    // Don't do anything if there's no connection!
    if (this->connectionType_ == SQLite::Connection::None) {
        this->setErrorMsg("No database connection exists!");
        return false;
    }

    // Finalize the previous query first
    this->finalizeQuery();

    // Prepare the query
    int result = sqlite3_prepare_v2(this->db, qry.c_str(), -1, &this->query, nullptr);
    if (result != SQLITE_OK || this->query == nullptr) {
        this->setErrorMsg();
        return false;
    }

    this->queryStatus = SQLite::Query::Ready;
    return true;
}

bool SQLite::bindInt(int col, int data) {
    // Check query status first
    if (this->queryStatus != SQLite::Query::Ready) {
        this->setErrorMsg("Unable to bind an integer to an unprepared query");
        return false;
    }

    // Now bind
    int result = sqlite3_bind_int(this->query, col+1, data);
    if (result != SQLITE_OK) {
        this->setErrorMsg();
        return false;
    }

    return true;
}

bool SQLite::bindString(int col, const std::string & data) {
    // Check query status first
    if (this->queryStatus != SQLite::Query::Ready) {
        this->setErrorMsg("Unable to bind a string to an unprepared query");
        return false;
    }

    // Now bind
    int result = sqlite3_bind_text(this->query, col+1, data.c_str(), -1, SQLITE_STATIC);
    if (result != SQLITE_OK) {
        this->setErrorMsg();
        return false;
    }

    return true;
}

bool SQLite::executeQuery() {
    // Check query status first
    if (this->queryStatus != SQLite::Query::Ready) {
        this->setErrorMsg("Can't execute an unprepared query");
        return false;
    }

    // Perform the query
    int result = sqlite3_step(this->query);
    bool ignore = (this->ignoreConstraints_ && (result & 0x00000011) == SQLITE_CONSTRAINT);
    if (result == SQLITE_DONE || ignore) {
        this->queryStatus = SQLite::Query::Finished;
    } else if (result == SQLITE_ROW) {
        this->queryStatus = SQLite::Query::Results;
    } else {
        this->queryStatus = SQLite::Query::Finished;
        this->setErrorMsg();
        return false;
    }

    return true;
}

bool SQLite::getInt(int col, int & data) {
    // Check query status first
    if (this->queryStatus != SQLite::Query::Results) {
        this->setErrorMsg("Unable to get an integer as no more rows are available");
        return false;
    }

    data = sqlite3_column_int(this->query, col);
    return true;
}

bool SQLite::getString(int col, std::string & data) {
    // Check query status first
    if (this->queryStatus != SQLite::Query::Results) {
        this->setErrorMsg("Unable to get a string as no more rows are available");
        return false;
    }

    const unsigned char * tmp = sqlite3_column_text(this->query, col);
    data = std::string(reinterpret_cast<const char *>(tmp));
    return true;
}

bool SQLite::nextRow() {
    // Check we have a row to move to
    if (this->queryStatus != SQLite::Query::Results) {
        this->setErrorMsg("Unable to move to next row as no more are available");
        return false;
    }

    // Attempt to move
    int result = sqlite3_step(this->query);
    if (result == SQLITE_ROW) {
        return true;
    } else {
        this->queryStatus = SQLite::Query::Finished;
    }

    return false;
}

SQLite::~SQLite() {
    // Cleans up both query and connection
    this->closeConnection();
}
