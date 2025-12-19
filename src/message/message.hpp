#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "db/db.hpp"
#include "pubsub/broker.hpp"

namespace message {

enum class Role {
  User,
  Assistant,
};

struct Message {
  std::string id;
  std::string session_id;
  Role role;
  std::string content;
  std::int64_t created_at = 0;
};

class Service {
 public:
  explicit Service(db::Db& db);

  Message create_user(const std::string& session_id, std::string content);
  Message create_assistant(const std::string& session_id, std::string content);

  std::vector<Message> list(const std::string& session_id);

  std::shared_ptr<pubsub::Channel<pubsub::Event<Message>>> subscribe();

 private:
  Message create(const std::string& session_id, Role role, std::string content);

  db::Db& db_;
  pubsub::Broker<Message> broker_;
};

std::string role_to_string(Role r);

}  // namespace message
