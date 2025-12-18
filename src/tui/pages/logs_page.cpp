#include "tui/pages/logs_page.hpp"

#include <algorithm>

#include <curses.h>

#include "tui/styles.hpp"

namespace tui {

LogsPage::LogsPage(logging::Logger& logger) : logger_(logger) {}

static int color_for(logging::Level lvl) {
  switch (lvl) {
    case logging::Level::Warn:
      return tui::styles::WarnPair;
    case logging::Level::Error:
      return tui::styles::ErrorPair;
    case logging::Level::Info:
    case logging::Level::Debug:
      return tui::styles::PrimaryPair;
  }
  return tui::styles::PrimaryPair;
}

void LogsPage::on_key(int ch) {
  auto msgs = logger_.list();
  if (msgs.empty()) {
    selected_ = 0;
    return;
  }

  if (ch == KEY_UP || ch == 'k') {
    selected_ = std::max(0, selected_ - 1);
  } else if (ch == KEY_DOWN || ch == 'j') {
    selected_ = std::min((int)msgs.size() - 1, selected_ + 1);
  }
}

void LogsPage::render(RenderCtx ctx) {
  mvprintw(2, 0, "Logs (use ↑/↓ or j/k)");

  auto msgs = logger_.list();
  int row = 4;
  int max_rows = ctx.height - row - 1;
  if (max_rows < 1) return;

  if (msgs.empty()) {
    mvprintw(row, 0, "No logs yet");
    return;
  }

  // Keep selection in range.
  selected_ = std::max(0, std::min((int)msgs.size() - 1, selected_));

  // Choose a window of messages that includes the selected row, and fits.
  int start = 0;
  if ((int)msgs.size() > max_rows) {
    start = std::max(0, selected_ - max_rows / 2);
    if (start + max_rows > (int)msgs.size()) start = (int)msgs.size() - max_rows;
  }

  for (int i = start; i < (int)msgs.size() && row < ctx.height - 1; i++) {
    const auto& m = msgs[(size_t)i];

    bool sel = (i == selected_);
    int cp = color_for(m.level);

    if (sel) attron(A_REVERSE);
    attron(COLOR_PAIR(cp));

    mvprintw(row++, 0, "[%s] %s", logging::level_to_string(m.level).c_str(), m.text.c_str());

    attroff(COLOR_PAIR(cp));
    if (sel) attroff(A_REVERSE);
  }
}

}  // namespace tui
