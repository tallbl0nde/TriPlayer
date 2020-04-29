#include "Database.hpp"
#include "sqlite3.h"
#include <stdlib.h>
#include <string.h>

// SQLite object
static sqlite3 * db;
// Command/statement
static sqlite3_stmt * cmd;

// TODO: add some logging

int dbOpen(int openRO) {
    sqlite3_close(db);
    if (sqlite3_open_v2("/switch/TriPlayer/music.db", &db, (openRO == 0 ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE), "unix-none") != SQLITE_OK) {
        return -1;
    }
    if (db == NULL) {
        return -2;
    }

    // Store journal in memory (otherwise file is unwritable)
    sqlite3_prepare_v2(db, "PRAGMA journal_mode=MEMORY;", -1, &cmd, NULL);
    if (cmd != NULL) {
        sqlite3_step(cmd);
    } else {
        return -3;
    }
    sqlite3_finalize(cmd);

    // Note foreign keys aren't required for these operations
    return 0;
}

int dbOpenReadOnly() {
    return dbOpen(0);
}

int dbOpenReadWrite() {
    return dbOpen(-1);
}

const char * dbGetPath(SongID id) {
    char * str = (char *) calloc(1, sizeof(char));

    if (db != NULL) {
        sqlite3_prepare_v2(db, "SELECT path FROM Songs WHERE id = ?;", -1, &cmd, NULL);
        if (cmd != NULL) {
            sqlite3_bind_int(cmd, 1, id);
            if (sqlite3_step(cmd) == SQLITE_ROW) {
                const char * ptr = (const char *) sqlite3_column_text(cmd, 0);
                const int len = sqlite3_column_bytes(cmd, 0);
                free((void *) str);
                str = (char *) malloc((len + 1) * sizeof(char));
                strcpy(str, ptr);
            }
        }
        sqlite3_finalize(cmd);
    }

    return str;
}

int dbIncrementPlays(SongID id) {
    return -1;
    // does nothing yet
}

void dbClose() {
    sqlite3_close(db);
}