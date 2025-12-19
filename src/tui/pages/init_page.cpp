#include "tui/pages/init_page.hpp"

#include <curses.h>

namespace tui {

void InitPage::render(RenderCtx ctx) {
  (void)ctx;
  mvprintw(2, 0, "Init page (placeholder)");
}

}  // namespace tui
