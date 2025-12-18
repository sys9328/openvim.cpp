#include "db/db.hpp"

#include <filesystem>
#include <stdexcept>

#include "db/migrate.hpp"

namespace db {

Db::Db(sqlite3* handle) : db_(handle) {}

Db::Db(Db&& other) noexcept : db_(other.db_) { other.db_ = nullptr; }

Db& Db::operator=(Db&& other) noexcept {
  if (this == &other) return *this;
  if (db_) sqlite3_close(db_);
  db_ = other.db_;
  other.db_ = nullptr;
  return *this;
}

Db::~Db() {
  if (db_) sqlite3_close(db_);
}

static void exec(sqlite3* db, const char* sql) {
  char* err = nullptr;
  int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    std::string msg = err ? err : "sqlite error";
    sqlite3_free(err);
    throw std::runtime_error(msg);
  }
}

Db connect(const std::string& data_dir) {
  namespace fs = std::filesystem;

  fs::create_directories(fs::path(data_dir));
  auto db_path = (fs::path(data_dir) / "openvim.db").string();

  sqlite3* raw = nullptr;
  int rc = sqlite3_open(db_path.c_str(), &raw);
  if (rc != SQLITE_OK) {
    std::string msg = raw ? sqlite3_errmsg(raw) : "sqlite3_open failed";
    if (raw) sqlite3_close(raw);
    throw std::runtime_error(msg);
  }

  // Basic pragmas (lightweight subset of Go's pragmas)
  exec(raw, "PRAGMA foreign_keys = ON;");
  exec(raw, "PRAGMA journal_mode = WAL;");

  // Apply SQL migrations from ./migrations
  // (commit-style parity with the Go repo, which uses migration files)
  apply_migrations(raw, "migrations");

  return Db(raw);
}

}  // namespace db
