#ifndef SQLITE_CLASS_HPP
#define SQLITE_CLASS_HPP

#include "sqlite3.h"
#include <string>

// A wrapper class for SQLite3 (only handles one query at a time).
// It takes care of a few things behind the scenes that SQLite3
// leaves up to the implementor!
class SQLite {
    public:
        // Enum for connection type
        enum class Connection {
            None,           // No connection open (or an error occurred trying to open)
            ReadOnly,       // Read-only connection
            ReadWrite       // Read-write connection
        };

    private:
        // Status of current query
        enum class Query {
            None,           // No query passed yet (or an error occurred creating one)
            Ready,          // Query is ready to be executed
            Results,        // Query was run and still has more rows available
            Finished        // Query has no more rows available and should be finalized
        };

        // Connection type
        Connection connectionType_;
        // SQLite database object
        sqlite3 * db;
        // Whether to ignore SQLITE_CONSTRAINT* result codes
        bool ignoreConstraints_;
        // Whether we are in a transaction
        bool inTransaction;
        // Path to file
        std::string path;
        // SQLite command
        sqlite3_stmt * query;
        // Status of query
        Query queryStatus;

        // Last logged error
        std::string errorMsg_;
        // Sets the above string (reads from SQLite) and also writes to application log
        void setErrorMsg(const std::string &);

        // Finalizes the current query
        void finalizeQuery();
        // Runs required PRAGMA statements
        bool prepare();

    public:
        // Constructor takes path to database file
        // It does not try to open a connection!
        SQLite(const std::string &);

        // Returns the last logged error message (blank if no error)
        std::string errorMsg();
        // Set whether to ignore constraint errors (don't interpret them as errors)
        void ignoreConstraints(bool);

        // Returns the current type of connection to the database file
        Connection connectionType();
        // Close an open connection (if there is one)
        // Does not throw an error if there is no connection
        void closeConnection();
        // Open a connection with the given type
        // Returns true if successful, false on an error
        bool openConnection(Connection);

        // Begin a transaction
        // Returns true on success, false on an error
        bool beginTransaction();
        // Commit and end the current transaction (rolls back if unable to commit!)
        // Returns true if successful (i.e. beginTransaction() was called), false on an error
        bool commitTransaction();
        // Rollback and end the current transaction
        // Returns true on success, false on an error
        bool rollbackTransaction();

        // Prepares the provided query (cleaned up automatically)
        // Returns true if successful, false on an error
        bool prepareQuery(const std::string &);
        // Functions to bind values to the given query
        // Parameters have order: (column number (starting from 0), data)
        // Returns true if successful, false on an error
        bool bindBool(int, const bool);
        bool bindInt(int, const int);
        bool bindString(int, const std::string &);

        // Performs the provided query on the database
        // Returns true if successful, false on an error
        bool executeQuery();
        // Accesses values given in the results (undefined if outside of range!)
        // Parameters have order: (column number (starting from 0), reference to fill with data)
        // Returns true if successful, false on an error
        bool getBool(int, bool &);
        bool getInt(int, int &);
        bool getString(int, std::string &);
        // Returns true if currently viewing a row, false otherwise
        bool hasRow();
        // Move to the next row in the results
        // Returns true if successful, false on an error (doesn't write message, most likely due to being at the end)
        bool nextRow();

        // Calls prepareQuery() and executeQuery() (does not allow binding obviously)
        bool prepareAndExecuteQuery(const std::string &);

        // Destructor ensures the database has been closed
        ~SQLite();
};

#endif