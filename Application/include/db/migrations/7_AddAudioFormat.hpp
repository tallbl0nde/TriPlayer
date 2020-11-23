#ifndef MIGRATION_7_HPP
#define MIGRATION_7_HPP

#include "SQLite.hpp"
#include <string>

// Migration 7
// Add audio_format column to Songs table
namespace Migration {
    std::string migrateTo7(SQLite *);
};

#endif