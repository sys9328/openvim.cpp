#include "tui/app.hpp"

#include <csignal>
#include <string>

#include <curses.h>

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

App::App(logging::Logger& logger) : logger_(logger) {
  init_page_ = std::make_unique<InitPage>();
  repl_page_ = std::make_unique<ReplPage>();
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

    // Back key: ESC
    if (ch == 27 /* ESC */) {
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
  if (has_colors()) {
    start_color();
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_YELLOW, -1);
    init_pair(3, COLOR_RED, -1);
  }
}

void App::shutdown_curses() {
  endwin();
}

void App::handle_resize() {
  int h = 0, w = 0;
  getmaxyx(stdscr, h, w);
  ctx_.height = h;
  ctx_.width = w;

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

  // Header
  attron(COLOR_PAIR(1));
  mvprintw(0, 0, "opencode-cpp (commit-1)  |  L: logs  ESC: back  q/Ctrl+C: quit");
  attroff(COLOR_PAIR(1));

  if (auto* p = page_ptr(current_, init_page_.get(), repl_page_.get(), logs_page_.get()); p != nullptr) {
    p->render(ctx_);
  }

  refresh();
}

}  // namespace tui
