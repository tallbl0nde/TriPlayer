#ifndef MIGRATION_1_HPP
#define MIGRATION_1_HPP

#include "SQLite.hpp"
#include <string>

// Migration 1
// Creates all initial tables (both for metadata and searching)
namespace Migration {
    std::string migrateTo1(SQLite *);
};

#endif