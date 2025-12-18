#pragma once

#include <memory>
#include <vector>

#include "logging/logger.hpp"
#include "tui/page.hpp"
#include "tui/pages/init_page.hpp"
#include "tui/pages/logs_page.hpp"
#include "tui/pages/repl_page.hpp"

namespace tui {

class App {
 public:
  explicit App(logging::Logger& logger);

  // Runs the blocking UI loop. Returns when the user quits.
  int run();

 private:
  void init_curses();
  void shutdown_curses();

  void handle_resize();
  void render();

  void move_to(PageId id);

  logging::Logger& logger_;
  RenderCtx ctx_{};

  PageId current_ = PageId::Repl;
  PageId previous_ = PageId::Repl;

  std::unique_ptr<InitPage> init_page_;
  std::unique_ptr<ReplPage> repl_page_;
  std::unique_ptr<LogsPage> logs_page_;

  bool running_ = true;
};

}  // namespace tui
