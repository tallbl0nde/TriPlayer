#ifndef DATABASE_H
#define DATABASE_H

#include "Types.h"

// Methods for interacting with database

// Open read-only connection to database
int dbOpenReadOnly();

// Open read-write connection to database
int dbOpenReadWrite();

// Get path for given id (returns empty string if not found)
// Returned string must be freed!
const char * dbGetPath(SongID);

// Increment play counter for given id
int dbIncrementPlays(SongID);

// Close connection to database
void dbClose();

#endif