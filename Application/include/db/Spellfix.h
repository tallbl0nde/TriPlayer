// This file isn't a part of the spellfix1 SQLite extension,
// I added it to avoid including the .c file
#ifndef SPELLFIX_H
#define SPELLFIX_H

#include "sqlite3.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initializes the spellfix extension
int sqlite3_spellfix_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi);

#ifdef __cplusplus
}
#endif

#endif