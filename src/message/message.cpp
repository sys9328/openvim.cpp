#include "message/message.hpp"

#include <chrono>
#include <random>
#include <stdexcept>

#include <sqlite3.h>

namespace message {

static std::string random_id() {
  static thread_local std::mt19937_64 rng{std::random_device{}()};
  std::uniform_int_distribution<std::uint64_t> dist;
  auto a = dist(rng);
  auto b = dist(rng);
  char buf[33];
  std::snprintf(buf, sizeof(buf), "%016llx%016llx", (unsigned long long)a, (unsigned long long)b);
  return std::string(buf);
}

static const char* role_db(Role r) {
  switch (r) {
    case Role::User:
      return "user";
    case Role::Assistant:
      return "assistant";
  }
  return "user";
}

Service::Service(db::Db& db) : db_(db) {}

Message Service::create_user(const std::string& session_id, std::string content) {
  return create(session_id, Role::User, std::move(content));
}

Message Service::create_assistant(const std::string& session_id, std::string content) {
  return create(session_id, Role::Assistant, std::move(content));
}

Message Service::create(const std::string& session_id, Role role, std::string content) {
  auto now = std::chrono::duration_cast<std::chrono::seconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count();

  Message m;
  m.id = random_id();
  m.session_id = session_id;
  m.role = role;
  m.content = std::move(content);
  m.created_at = (std::int64_t)now;

  sqlite3_stmt* stmt = nullptr;
  const char* sql =
      "INSERT INTO messages(id, session_id, role, content, created_at) VALUES(?, ?, ?, ?, ?);";
  if (sqlite3_prepare_v2(db_.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error(sqlite3_errmsg(db_.get()));
  }

  sqlite3_bind_text(stmt, 1, m.id.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, m.session_id.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, role_db(role), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 4, m.content.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt, 5, m.created_at);

  int rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if (rc != SQLITE_DONE) throw std::runtime_error(sqlite3_errmsg(db_.get()));

  broker_.publish(pubsub::EventType::Created, m);

  return m;
}

std::vector<Message> Service::list(const std::string& session_id) {
  std::vector<Message> out;

  sqlite3_stmt* stmt = nullptr;
  const char* sql =
      "SELECT id, session_id, role, content, created_at FROM messages WHERE session_id = ? "
      "ORDER BY created_at ASC;";
  if (sqlite3_prepare_v2(db_.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error(sqlite3_errmsg(db_.get()));
  }
  sqlite3_bind_text(stmt, 1, session_id.c_str(), -1, SQLITE_TRANSIENT);

  while (true) {
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      Message m;
      m.id = (const char*)sqlite3_column_text(stmt, 0);
      m.session_id = (const char*)sqlite3_column_text(stmt, 1);
      auto role = (const char*)sqlite3_column_text(stmt, 2);
      m.role = (role && std::string(role) == "assistant") ? Role::Assistant : Role::User;
      m.content = (const char*)sqlite3_column_text(stmt, 3);
      m.created_at = sqlite3_column_int64(stmt, 4);
      out.push_back(std::move(m));
      continue;
    }
    if (rc == SQLITE_DONE) break;
    sqlite3_finalize(stmt);
    throw std::runtime_error(sqlite3_errmsg(db_.get()));
  }

  sqlite3_finalize(stmt);
  return out;
}

std::string role_to_string(Role r) {
  switch (r) {
    case Role::User:
      return "user";
    case Role::Assistant:
      return "assistant";
  }
  return "user";
}

std::shared_ptr<pubsub::Channel<pubsub::Event<Message>>> Service::subscribe() {
  return broker_.subscribe();
}

}  // namespace message
