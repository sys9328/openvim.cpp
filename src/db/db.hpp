#pragma once

#include <sqlite3.h>

#include <string>

namespace db {

class Db {
 public:
  Db() = default;
  explicit Db(sqlite3* handle);
  Db(const Db&) = delete;
  Db& operator=(const Db&) = delete;
  Db(Db&& other) noexcept;
  Db& operator=(Db&& other) noexcept;
  ~Db();

  sqlite3* get() const { return db_; }
  explicit operator bool() const { return db_ != nullptr; }

 private:
  sqlite3* db_ = nullptr;
};

// Opens (and creates) the DB at <data_dir>/openvim.db, ensures schema exists.
Db connect(const std::string& data_dir);

}  // namespace db
