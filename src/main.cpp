#include "config.hpp"
#include "db/db.hpp"
#include "llm/llm.hpp"
#include "logging/logger.hpp"
#include "message/message.hpp"
#include "session/session.hpp"
#include "tui/app.hpp"

#include <atomic>
#include <chrono>
#include <thread>

int main(int argc, char** argv) {
  auto cfg = config::parse_args_or_exit(argc, argv);

  logging::Logger log;
  log.info("Starting openvim...");
  if (cfg.debug) log.debug("Debug enabled");

  db::Db db;
  try {
    db = db::connect(cfg.data_dir);
    log.info("DB ready at data dir: " + cfg.data_dir);
  } catch (const std::exception& e) {
    log.error(std::string("DB init failed: ") + e.what());
    return 1;
  }

  session::Service sessions(db);
  message::Service messages(db);
  llm::Service llm(log, messages);

  // Create an initial session if none exist.
  std::string active_session_id;
  try {
    auto existing = sessions.list();
    if (existing.empty()) {
      auto s = sessions.create("Welcome session");
      active_session_id = s.id;
    } else {
      active_session_id = existing.front().id;
    }
  } catch (const std::exception& e) {
    log.warn(std::string("Session bootstrap failed: ") + e.what());
  }

  // If DB has no session for some reason, create one.
  if (active_session_id.empty()) {
    auto s = sessions.create("Welcome session");
    active_session_id = s.id;
  }

  std::atomic<bool> running{true};
  std::thread producer([&] {
    using namespace std::chrono_literals;
    for (int i = 1; running && i <= 3; i++) {
      std::this_thread::sleep_for(700ms);
      if (cfg.debug) log.debug("tick " + std::to_string(i));
    }
  });

  auto titles_provider = [&sessions]() -> std::vector<std::string> {
    std::vector<std::string> out;
    for (auto& s : sessions.list()) out.push_back(s.title);
    return out;
  };

  auto messages_provider = [&messages, &active_session_id]() -> std::vector<std::string> {
    std::vector<std::string> out;
    for (auto& m : messages.list(active_session_id)) {
      out.push_back(message::role_to_string(m.role) + ": " + m.content);
    }
    return out;
  };

  auto send = [&messages, &llm, &active_session_id](std::string content) {
    messages.create_user(active_session_id, content);
    llm.send_request(active_session_id, content);
  };

  tui::App app(log, titles_provider, messages_provider, send);
  int rc = app.run();

  running = false;
  if (producer.joinable()) producer.join();
  return rc;
}
