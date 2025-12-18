#include "tui/pages/repl_page.hpp"

#include <curses.h>

namespace tui {

void ReplPage::render(RenderCtx ctx) {
  (void)ctx;
  mvprintw(2, 0, "REPL page (placeholder)");
  mvprintw(3, 0, "In later commits we'll implement input + message threads.");
  mvprintw(5, 0, "Press L to view logs.");
}

}  // namespace tui
