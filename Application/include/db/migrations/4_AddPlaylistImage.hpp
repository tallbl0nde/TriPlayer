#ifndef MIGRATION_4_HPP
#define MIGRATION_4_HPP

#include "SQLite.hpp"
#include <string>

// Migration 4
// Adds an image column to the Playlists table
namespace Migration {
    std::string migrateTo4(SQLite *);
};

#endif