#include "db/migrate.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace db {

static void exec(sqlite3* db, const std::string& sql) {
  char* err = nullptr;
  int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    std::string msg = err ? err : "sqlite error";
    sqlite3_free(err);
    throw std::runtime_error(msg);
  }
}

static std::string read_file(const std::filesystem::path& p) {
  std::ifstream in(p, std::ios::in | std::ios::binary);
  if (!in) throw std::runtime_error("failed to read migration file: " + p.string());
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

static std::set<std::string> load_applied(sqlite3* db) {
  exec(db,
       "CREATE TABLE IF NOT EXISTS schema_migrations ("
       "  version TEXT PRIMARY KEY,"
       "  applied_at INTEGER NOT NULL"
       ");");

  std::set<std::string> out;

  sqlite3_stmt* stmt = nullptr;
  const char* sql = "SELECT version FROM schema_migrations;";
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error(sqlite3_errmsg(db));
  }

  while (true) {
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      auto v = (const char*)sqlite3_column_text(stmt, 0);
      if (v) out.insert(v);
      continue;
    }
    if (rc == SQLITE_DONE) break;
    sqlite3_finalize(stmt);
    throw std::runtime_error(sqlite3_errmsg(db));
  }

  sqlite3_finalize(stmt);
  return out;
}

static void mark_applied(sqlite3* db, const std::string& version) {
  auto now = std::chrono::duration_cast<std::chrono::seconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count();

  sqlite3_stmt* stmt = nullptr;
  const char* sql = "INSERT INTO schema_migrations(version, applied_at) VALUES(?, ?);";
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error(sqlite3_errmsg(db));
  }

  sqlite3_bind_text(stmt, 1, version.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt, 2, (sqlite3_int64)now);

  int rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if (rc != SQLITE_DONE) throw std::runtime_error(sqlite3_errmsg(db));
}

static std::string version_from_filename(const std::filesystem::path& p) {
  // e.g. 0001_initial.up.sql -> 0001_initial
  auto name = p.filename().string();
  auto pos = name.find(".up.sql");
  if (pos == std::string::npos) return "";
  return name.substr(0, pos);
}

void apply_migrations(sqlite3* db, const std::string& migrations_dir) {
  namespace fs = std::filesystem;

  auto applied = load_applied(db);

  std::vector<fs::path> ups;
  for (auto const& entry : fs::directory_iterator(fs::path(migrations_dir))) {
    if (!entry.is_regular_file()) continue;
    auto p = entry.path();
    if (p.filename().string().ends_with(".up.sql")) ups.push_back(p);
  }

  std::sort(ups.begin(), ups.end());

  for (auto const& file : ups) {
    auto ver = version_from_filename(file);
    if (ver.empty()) continue;
    if (applied.contains(ver)) continue;

    // Apply each migration in a transaction.
    exec(db, "BEGIN;");
    try {
      exec(db, read_file(file));
      mark_applied(db, ver);
      exec(db, "COMMIT;");
    } catch (...) {
      exec(db, "ROLLBACK;");
      throw;
    }
  }
}

}  // namespace db
