#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "db/db.hpp"
#include "pubsub/broker.hpp"

namespace session {

struct Session {
  std::string id;
  std::string title;
  std::int64_t created_at = 0;
};

class Service {
 public:
  explicit Service(db::Db& db);

  Session create(std::string title);
  std::vector<Session> list();
  Session get(const std::string& id);
  void update_title(const std::string& id, const std::string& title);

  std::shared_ptr<pubsub::Channel<pubsub::Event<Session>>> subscribe();

 private:
  db::Db& db_;
  pubsub::Broker<Session> broker_;
};

}  // namespace session
