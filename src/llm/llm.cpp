#include "llm/llm.hpp"

#include <chrono>

namespace llm {

Service::Service(logging::Logger& log, message::Service& messages) : log_(log), messages_(messages) {}

void Service::send_request(const std::string& session_id, const std::string& content) {
  broker_.publish(pubsub::EventType::Created, AgentEvent{AgentEventType::Request, session_id, content});
  std::thread(&Service::worker, this, session_id, content).detach();
}

void Service::worker(std::string session_id, std::string content) {
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(300ms);

  try {
    // Very small placeholder "agent": just echoes for now.
    auto reply = std::string("(stub agent) You said: ") + content;
    messages_.create_assistant(session_id, reply);
    broker_.publish(pubsub::EventType::Created, AgentEvent{AgentEventType::Response, session_id, reply});
  } catch (const std::exception& e) {
    log_.error(std::string("LLM error: ") + e.what());
    broker_.publish(pubsub::EventType::Created,
                    AgentEvent{AgentEventType::Error, session_id, std::string("LLM error: ") + e.what()});
  }
}

std::shared_ptr<pubsub::Channel<pubsub::Event<AgentEvent>>> Service::subscribe() {
  return broker_.subscribe();
}

}  // namespace llm
