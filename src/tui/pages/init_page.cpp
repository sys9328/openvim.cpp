#include "tui/pages/init_page.hpp"

#include <curses.h>

namespace tui {

void InitPage::render(RenderCtx ctx) {
  (void)ctx;
  mvprintw(2, 0, "Init page (placeholder)");
  mvprintw(3, 0, "This roughly corresponds to internal/tui/page/init.go in the first Go commit.");
}

}  // namespace tui
