#pragma once

#include <memory>
#include <vector>

#include "logging/logger.hpp"
#include "message/message.hpp"
#include "permission/permission.hpp"
#include "pubsub/broker.hpp"
#include "session/session.hpp"
#include "tui/components/permission_dialog.hpp"
#include "tui/page.hpp"
#include "tui/pages/init_page.hpp"
#include "tui/pages/logs_page.hpp"
#include "tui/pages/repl_page.hpp"

namespace tui {

class App {
 public:
  App(logging::Logger& logger,
      ReplPage::SessionsProvider sessions_provider,
      ReplPage::MessagesProvider messages_provider,
      ReplPage::SendFn send,
      std::shared_ptr<pubsub::Channel<pubsub::Event<message::Message>>> message_subscriber,
      std::shared_ptr<pubsub::Channel<pubsub::Event<session::Session>>> session_subscriber,
      std::shared_ptr<pubsub::Channel<pubsub::Event<permission::PermissionRequest>>> permission_subscriber);

  // Runs the blocking UI loop. Returns when the user quits.
  int run();

 private:
  void init_curses();
  void shutdown_curses();

  void handle_resize();
  void render();

  void move_to(PageId id);
  void toggle_help();

  logging::Logger& logger_;
  std::shared_ptr<pubsub::Channel<pubsub::Event<message::Message>>> message_subscriber_;
  std::shared_ptr<pubsub::Channel<pubsub::Event<session::Session>>> session_subscriber_;
  std::shared_ptr<pubsub::Channel<pubsub::Event<permission::PermissionRequest>>> permission_subscriber_;
  RenderCtx ctx_{};

  PageId current_ = PageId::Repl;
  PageId previous_ = PageId::Repl;

  std::unique_ptr<InitPage> init_page_;
  std::unique_ptr<ReplPage> repl_page_;
  std::unique_ptr<LogsPage> logs_page_;

  std::unique_ptr<PermissionDialog> permission_dialog_;

  bool show_help_ = false;
  static constexpr int k_status_height_ = 1;
  static constexpr int k_help_height_ = 6;

  bool running_ = true;
  std::atomic<bool> needs_render_{false};
};

}  // namespace tui
