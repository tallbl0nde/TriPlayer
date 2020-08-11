#ifndef MIGRATION_5_HPP
#define MIGRATION_5_HPP

#include "SQLite.hpp"
#include <string>

// Migration 5
// Adds artist and album columns to FtsSongs table
namespace Migration {
    std::string migrateTo5(SQLite *);
};

#endif