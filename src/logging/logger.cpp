#include "logging/logger.hpp"

namespace logging {

Logger::Logger() = default;

void Logger::debug(std::string msg) { log(Level::Debug, std::move(msg)); }
void Logger::info(std::string msg) { log(Level::Info, std::move(msg)); }
void Logger::warn(std::string msg) { log(Level::Warn, std::move(msg)); }
void Logger::error(std::string msg) { log(Level::Error, std::move(msg)); }

void Logger::log(Level lvl, std::string msg) {
  Message m{lvl, std::move(msg), std::chrono::system_clock::now()};
  {
    std::lock_guard<std::mutex> lk(mu_);
    messages_.push_back(m);
  }
  broker_.publish(pubsub::EventType::Created, m);
}

std::vector<Message> Logger::list() const {
  std::lock_guard<std::mutex> lk(mu_);
  return messages_;
}

std::shared_ptr<pubsub::Channel<pubsub::Event<Message>>> Logger::subscribe() {
  return broker_.subscribe();
}

std::string level_to_string(Level lvl) {
  switch (lvl) {
    case Level::Debug:
      return "DEBUG";
    case Level::Info:
      return "INFO";
    case Level::Warn:
      return "WARN";
    case Level::Error:
      return "ERROR";
  }
  return "INFO";
}

}  // namespace logging
