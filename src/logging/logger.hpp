#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <vector>

#include "pubsub/broker.hpp"

namespace logging {

enum class Level {
  Debug,
  Info,
  Warn,
  Error,
};

struct Message {
  Level level;
  std::string text;
  std::chrono::system_clock::time_point at;
};

class Logger {
 public:
  Logger();

  void debug(std::string msg);
  void info(std::string msg);
  void warn(std::string msg);
  void error(std::string msg);

  std::vector<Message> list() const;

  std::shared_ptr<pubsub::Channel<pubsub::Event<Message>>> subscribe();

 private:
  void log(Level lvl, std::string msg);

  mutable std::mutex mu_;
  std::vector<Message> messages_;
  pubsub::Broker<Message> broker_;
};

std::string level_to_string(Level lvl);

}  // namespace logging
