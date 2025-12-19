#include "tui/pages/repl_page.hpp"

#include <curses.h>

namespace tui {

ReplPage::ReplPage(SessionsProvider sessions_provider, MessagesProvider messages_provider, SendFn send)
    : sessions_provider_(std::move(sessions_provider)),
      messages_provider_(std::move(messages_provider)),
      send_(std::move(send)) {}

void ReplPage::on_key(int ch) {
  // Enter sends message.
  if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
    if (!input_.empty() && send_) {
      send_(input_);
      input_.clear();
    }
    return;
  }

  // Backspace edits.
  if (ch == KEY_BACKSPACE || ch == 127) {
    if (!input_.empty()) input_.pop_back();
    return;
  }

  // Basic printable characters.
  if (ch >= 32 && ch <= 126) {
    input_.push_back((char)ch);
  }
}

void ReplPage::render(RenderCtx ctx) {
  int row = 2;

  mvprintw(row++, 0, "REPL");
  mvprintw(row++, 0, "Type a message and press Enter (stub agent will reply).");

  // Sessions
  row++;
  mvprintw(row++, 0, "Sessions:");
  if (sessions_provider_) {
    auto titles = sessions_provider_();
    if (titles.empty()) {
      mvprintw(row++, 0, "  (none)");
    } else {
      for (int i = 0; i < (int)titles.size() && row < ctx.height - 6; i++) {
        mvprintw(row++, 0, "  - %s", titles[(size_t)i].c_str());
      }
    }
  }

  // Messages
  row++;
  mvprintw(row++, 0, "Messages:");
  if (messages_provider_) {
    auto msgs = messages_provider_();
    if (msgs.empty()) {
      mvprintw(row++, 0, "  (no messages)");
    } else {
      int max_rows = ctx.height - row - 4;
      int start = 0;
      if ((int)msgs.size() > max_rows) start = (int)msgs.size() - max_rows;
      for (int i = start; i < (int)msgs.size() && row < ctx.height - 3; i++) {
        mvprintw(row++, 0, "%.*s", ctx.width - 1, msgs[(size_t)i].c_str());
      }
    }
  }

  // Input box
  int input_y = ctx.height - 2;
  if (input_y < row + 1) input_y = row + 1;
  mvprintw(input_y, 0, "> %.*s", ctx.width - 4, input_.c_str());

  mvprintw(ctx.height - 1, 0, "L: logs  ?: help  q: quit");
}

}  // namespace tui
