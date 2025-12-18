#include "logging/logger.hpp"
#include "tui/app.hpp"

#include <atomic>
#include <chrono>
#include <thread>

int main() {
  logging::Logger log;
  log.info("Starting opencode-cpp...");
  log.info("Setting up subscriptions...");

  // In the first Go commit, logs are forwarded as tea.Msg into Bubble Tea.
  // Here we keep it simple: pages directly read logger history, and we also
  // generate a few log lines to prove plumbing works.
  std::atomic<bool> running{true};
  std::thread producer([&] {
    using namespace std::chrono_literals;
    for (int i = 1; running && i <= 5; i++) {
      std::this_thread::sleep_for(600ms);
      log.info("tick " + std::to_string(i));
    }
    log.warn("Press L to view logs; q to quit.");
  });

  tui::App app(log);
  int rc = app.run();

  running = false;
  if (producer.joinable()) producer.join();
  return rc;
}
