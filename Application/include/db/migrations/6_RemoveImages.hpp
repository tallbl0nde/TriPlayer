#ifndef MIGRATION_6_HPP
#define MIGRATION_6_HPP

#include "SQLite.hpp"
#include <string>

// Migration 6
// Create triggers to remove images stored on disk
namespace Migration {
    std::string migrateTo6(SQLite *);
};

#endif