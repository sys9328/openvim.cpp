#include "session/session.hpp"

#include <chrono>
#include <random>
#include <stdexcept>

#include <sqlite3.h>

namespace session {

static std::string random_id() {
  static thread_local std::mt19937_64 rng{std::random_device{}()};
  std::uniform_int_distribution<std::uint64_t> dist;
  auto a = dist(rng);
  auto b = dist(rng);
  char buf[33];
  std::snprintf(buf, sizeof(buf), "%016llx%016llx", (unsigned long long)a, (unsigned long long)b);
  return std::string(buf);
}

Service::Service(db::Db& db) : db_(db) {}

Session Service::create(std::string title) {
  auto now = std::chrono::duration_cast<std::chrono::seconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count();

  Session s;
  s.id = random_id();
  s.title = std::move(title);
  s.created_at = (std::int64_t)now;

  sqlite3_stmt* stmt = nullptr;
  const char* sql = "INSERT INTO sessions(id, title, created_at) VALUES(?, ?, ?);";
  if (sqlite3_prepare_v2(db_.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error(sqlite3_errmsg(db_.get()));
  }

  sqlite3_bind_text(stmt, 1, s.id.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, s.title.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt, 3, s.created_at);

  int rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  if (rc != SQLITE_DONE) {
    throw std::runtime_error(sqlite3_errmsg(db_.get()));
  }

  return s;
}

std::vector<Session> Service::list() {
  std::vector<Session> out;

  sqlite3_stmt* stmt = nullptr;
  const char* sql = "SELECT id, title, created_at FROM sessions ORDER BY created_at DESC;";
  if (sqlite3_prepare_v2(db_.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error(sqlite3_errmsg(db_.get()));
  }

  while (true) {
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      Session s;
      s.id = (const char*)sqlite3_column_text(stmt, 0);
      s.title = (const char*)sqlite3_column_text(stmt, 1);
      s.created_at = sqlite3_column_int64(stmt, 2);
      out.push_back(std::move(s));
      continue;
    }
    if (rc == SQLITE_DONE) break;
    sqlite3_finalize(stmt);
    throw std::runtime_error(sqlite3_errmsg(db_.get()));
  }

  sqlite3_finalize(stmt);
  return out;
}

Session Service::get(const std::string& id) {
  sqlite3_stmt* stmt = nullptr;
  const char* sql = "SELECT id, title, created_at FROM sessions WHERE id = ? LIMIT 1;";
  if (sqlite3_prepare_v2(db_.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error(sqlite3_errmsg(db_.get()));
  }
  sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

  int rc = sqlite3_step(stmt);
  if (rc != SQLITE_ROW) {
    sqlite3_finalize(stmt);
    throw std::runtime_error("session not found");
  }

  Session s;
  s.id = (const char*)sqlite3_column_text(stmt, 0);
  s.title = (const char*)sqlite3_column_text(stmt, 1);
  s.created_at = sqlite3_column_int64(stmt, 2);

  sqlite3_finalize(stmt);
  return s;
}

void Service::update_title(const std::string& id, const std::string& title) {
  sqlite3_stmt* stmt = nullptr;
  const char* sql = "UPDATE sessions SET title = ? WHERE id = ?;";
  if (sqlite3_prepare_v2(db_.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error(sqlite3_errmsg(db_.get()));
  }
  sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, id.c_str(), -1, SQLITE_TRANSIENT);

  int rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if (rc != SQLITE_DONE) {
    throw std::runtime_error(sqlite3_errmsg(db_.get()));
  }

  // Publish update event
  Session updated = get(id);
  broker_.publish(pubsub::EventType::Updated, updated);
}

std::shared_ptr<pubsub::Channel<pubsub::Event<Session>>> Service::subscribe() {
  return broker_.subscribe();
}

}  // namespace session
