#pragma once

#include <functional>
#include <string>
#include <thread>

#include "logging/logger.hpp"
#include "message/message.hpp"
#include "pubsub/broker.hpp"

namespace llm {

enum class AgentEventType {
  Request,
  Response,
  Error,
};

struct AgentEvent {
  AgentEventType type;
  std::string session_id;
  std::string content;
};

class Service {
 public:
  Service(logging::Logger& log, message::Service& messages);

  // Fire-and-forget async request. Will create an assistant message and publish a Response event.
  void send_request(const std::string& session_id, const std::string& content);

  std::shared_ptr<pubsub::Channel<pubsub::Event<AgentEvent>>> subscribe();

 private:
  void worker(std::string session_id, std::string content);

  logging::Logger& log_;
  message::Service& messages_;
  pubsub::Broker<AgentEvent> broker_;
};

}  // namespace llm
