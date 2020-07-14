#ifndef MIGRATION_2_HPP
#define MIGRATION_2_HPP

#include "SQLite.hpp"
#include <string>

// Migration 2
// Adds more metadata columns to the Artists table to handle an external image
namespace Migration {
    std::string migrateTo2(SQLite *);
};

#endif