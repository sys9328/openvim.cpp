#include "tui/pages/logs_page.hpp"

#include <curses.h>

namespace tui {

LogsPage::LogsPage(logging::Logger& logger) : logger_(logger) {}

static int color_for(logging::Level lvl) {
  switch (lvl) {
    case logging::Level::Info:
      return 1;
    case logging::Level::Warn:
      return 2;
    case logging::Level::Error:
      return 3;
    case logging::Level::Debug:
      return 1;
  }
  return 1;
}

void LogsPage::render(RenderCtx ctx) {
  mvprintw(2, 0, "Logs");

  auto msgs = logger_.list();
  int row = 4;
  int max_rows = ctx.height - row - 1;
  if (max_rows < 1) return;

  // Show last N messages.
  int start = 0;
  if ((int)msgs.size() > max_rows) start = (int)msgs.size() - max_rows;

  for (int i = start; i < (int)msgs.size(); i++) {
    const auto& m = msgs[(size_t)i];
    int cp = color_for(m.level);
    attron(COLOR_PAIR(cp));
    mvprintw(row++, 0, "[%s] %s", logging::level_to_string(m.level).c_str(), m.text.c_str());
    attroff(COLOR_PAIR(cp));
  }
}

}  // namespace tui
