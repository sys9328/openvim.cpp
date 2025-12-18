#pragma once

#include <sqlite3.h>

#include <string>

namespace db {

// Applies all *.up.sql migrations in the given directory.
// Uses a schema_migrations table to ensure each version runs once.
void apply_migrations(sqlite3* db, const std::string& migrations_dir);

}  // namespace db
