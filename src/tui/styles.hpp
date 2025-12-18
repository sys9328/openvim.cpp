#pragma once

#include <curses.h>

namespace tui::styles {

// ncurses color pair IDs
static constexpr short PrimaryPair = 1;
static constexpr short SecondaryPair = 2;
static constexpr short WarnPair = 3;
static constexpr short ErrorPair = 4;

inline void init_colors() {
  if (!has_colors()) return;
  start_color();
  use_default_colors();

  // Map "Primary"/"Secondary" to the closest standard terminal colors.
  init_pair(PrimaryPair, COLOR_CYAN, -1);
  init_pair(SecondaryPair, COLOR_MAGENTA, -1);
  init_pair(WarnPair, COLOR_YELLOW, -1);
  init_pair(ErrorPair, COLOR_RED, -1);
}

}  // namespace tui::styles
