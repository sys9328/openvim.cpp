#include "tui/app.hpp"

#include <csignal>
#include <string>

#include <curses.h>

#include "tui/styles.hpp"

namespace tui {

static bool g_resize = false;

static void on_winch(int) {
  g_resize = true;
}

static Page* page_ptr(PageId id, InitPage* init, ReplPage* repl, LogsPage* logs) {
  switch (id) {
    case PageId::Init:
      return init;
    case PageId::Repl:
      return repl;
    case PageId::Logs:
      return logs;
  }
  return repl;
}

App::App(logging::Logger& logger,
         ReplPage::SessionsProvider sessions_provider,
         ReplPage::MessagesProvider messages_provider,
         ReplPage::SendFn send)
    : logger_(logger) {
  init_page_ = std::make_unique<InitPage>();
  repl_page_ = std::make_unique<ReplPage>(std::move(sessions_provider), std::move(messages_provider), std::move(send));
  logs_page_ = std::make_unique<LogsPage>(logger_);
}

int App::run() {
  init_curses();

  // Install SIGWINCH handler for resize.
  std::signal(SIGWINCH, on_winch);

  handle_resize();
  render();

  while (running_) {
    if (g_resize) {
      g_resize = false;
      handle_resize();
      render();
    }

    int ch = getch();
    if (ch == ERR) {
      continue;
    }

    // Quit keys: q or Ctrl+C
    if (ch == 'q' || ch == 3 /* Ctrl+C */) {
      running_ = false;
      break;
    }

    // Toggle help: ?
    if (ch == '?') {
      toggle_help();
      render();
      continue;
    }

    // Close help: ESC
    if (ch == 27 /* ESC */) {
      if (show_help_) {
        toggle_help();
        render();
        continue;
      }
      // Back to previous page (commit-1 behavior kept)
      if (previous_ != current_) {
        move_to(previous_);
        render();
      }
      continue;
    }

    // Back key: backspace
    if (ch == KEY_BACKSPACE || ch == 127 /* DEL */) {
      if (previous_ != current_) {
        move_to(previous_);
        render();
      }
      continue;
    }

    // Logs key: L
    if (ch == 'L') {
      move_to(PageId::Logs);
      render();
      continue;
    }

    // Forward to current page.
    if (auto* p = page_ptr(current_, init_page_.get(), repl_page_.get(), logs_page_.get()); p != nullptr) {
      p->on_key(ch);
    }

    // Also handle KEY_RESIZE for some terminals.
    if (ch == KEY_RESIZE) {
      handle_resize();
    }

    render();
  }

  shutdown_curses();
  return 0;
}

void App::init_curses() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, FALSE);

  // Try to enable colors (best-effort).
  tui::styles::init_colors();
}

void App::shutdown_curses() {
  endwin();
}

void App::handle_resize() {
  int h = 0, w = 0;
  getmaxyx(stdscr, h, w);
  ctx_.width = w;

  // Reserve bottom line for status bar, and optionally reserve additional lines for help.
  int usable_h = h - k_status_height_;
  if (show_help_) usable_h -= k_help_height_;
  if (usable_h < 1) usable_h = 1;

  ctx_.height = usable_h;

  if (auto* p = page_ptr(current_, init_page_.get(), repl_page_.get(), logs_page_.get()); p != nullptr) {
    p->on_resize(ctx_);
  }
}

void App::move_to(PageId id) {
  previous_ = current_;
  current_ = id;
  if (auto* p = page_ptr(current_, init_page_.get(), repl_page_.get(), logs_page_.get()); p != nullptr) {
    p->on_resize(ctx_);
  }
}

void App::render() {
  erase();

  // Header (part of the page area)
  attron(COLOR_PAIR(tui::styles::PrimaryPair));
  mvprintw(0, 0, "openvim (commit-3)  |  L: logs  ?: help  ESC: close/back  q: quit");
  attroff(COLOR_PAIR(tui::styles::PrimaryPair));

  if (auto* p = page_ptr(current_, init_page_.get(), repl_page_.get(), logs_page_.get()); p != nullptr) {
    p->render(ctx_);
  }

  int term_h = 0, term_w = 0;
  getmaxyx(stdscr, term_h, term_w);

  // Help panel sits above status bar.
  if (show_help_) {
    int top = term_h - k_status_height_ - k_help_height_;
    if (top < 1) top = 1;

    // Simple bordered box.
    attron(A_DIM);
    for (int r = 0; r < k_help_height_; r++) {
      int y = top + r;
      if (y >= term_h - k_status_height_) break;
      mvhline(y, 0, ' ', term_w);
    }
    attroff(A_DIM);

    mvprintw(top + 0, 0, "Help");
    mvprintw(top + 1, 0, "  L           logs");
    mvprintw(top + 2, 0, "  ?           toggle help");
    mvprintw(top + 3, 0, "  ESC         close help (or go back if help is closed)");
    mvprintw(top + 4, 0, "  Backspace   back");
    mvprintw(top + 5, 0, "  q / Ctrl+C  quit");
  }

  // Status bar (always bottom line)
  int status_y = term_h - 1;
  if (status_y >= 0) {
    auto msgs = logger_.list();
    std::string status = "Ready";
    if (!msgs.empty()) {
      const auto& last = msgs.back();
      status = "[" + logging::level_to_string(last.level) + "] " + last.text;
    }
    // Clear the line and print.
    attron(A_REVERSE);
    mvhline(status_y, 0, ' ', term_w);
    mvprintw(status_y, 0, "%.*s", term_w, status.c_str());
    attroff(A_REVERSE);
  }

  refresh();
}

void App::toggle_help() {
  show_help_ = !show_help_;
  handle_resize();
}

}  // namespace tui
