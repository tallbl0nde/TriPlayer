// This file isn't a part of the okapi_mb25.h SQLite extension,
// I added it to avoid including the .c file
#ifndef OKAPI_BM25_H
#define OKAPI_BM25_H

#include "sqlite3.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initializes the spellfix extension
int sqlite3_okapi_bm25_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi);

#ifdef __cplusplus
}
#endif

#endif