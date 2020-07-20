#ifndef MIGRATION_3_HPP
#define MIGRATION_3_HPP

#include "SQLite.hpp"
#include <string>

// Migration 3
// Adds more metadata columns to the Albums table to handle an external image
namespace Migration {
    std::string migrateTo3(SQLite *);
};

#endif