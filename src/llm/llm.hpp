#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "logging/logger.hpp"
#include "message/message.hpp"
#include "pubsub/broker.hpp"
#include "tools/tool.hpp"

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

  // Generate a title for the session based on the first message content.
  std::string generate_title(const std::string& content);

  // Get available tools
  const std::vector<std::unique_ptr<tools::BaseTool>>& tools() const;

 private:
  void worker(std::string session_id, std::string content);

  logging::Logger& log_;
  message::Service& messages_;
  pubsub::Broker<AgentEvent> broker_;
  std::vector<std::unique_ptr<tools::BaseTool>> tools_;
  std::string working_dir_ = ".";
};

}  // namespace llm
