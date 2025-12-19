#include "permission_dialog.hpp"

#include <sstream>
#include <curses.h>

namespace tui {

PermissionDialog::PermissionDialog(const permission::PermissionRequest& request, const std::string& content)
    : permission_(request), content_(content), selected_option_(0), has_response_(false), response_action_(PermissionAction::Deny) {}

void PermissionDialog::render(WINDOW* win, int width, int height) {
  // Draw border
  box(win, 0, 0);
  
  // Title
  mvwprintw(win, 1, 2, "Permission Required");
  
  // Content
  int content_y = 3;
  std::istringstream iss(content_);
  std::string line;
  while (std::getline(iss, line) && content_y < height - 3) {
    mvwprintw(win, content_y++, 2, "%.*s", width - 4, line.c_str());
  }
  
  // Options
  int options_y = height - 3;
  for (size_t i = 0; i < options_.size(); ++i) {
    if (static_cast<int>(i) == selected_option_) {
      wattron(win, A_REVERSE);
    }
    mvwprintw(win, options_y + i, 2, "%s", options_[i].c_str());
    if (static_cast<int>(i) == selected_option_) {
      wattroff(win, A_REVERSE);
    }
  }
  
  wrefresh(win);
}

bool PermissionDialog::handle_input(int ch) {
  switch (ch) {
    case KEY_UP:
      if (selected_option_ > 0) selected_option_--;
      return true;
    case KEY_DOWN:
      if (selected_option_ < static_cast<int>(options_.size()) - 1) selected_option_++;
      return true;
    case '\n':
    case KEY_ENTER:
      has_response_ = true;
      switch (selected_option_) {
        case 0: response_action_ = PermissionAction::Allow; break;
        case 1: response_action_ = PermissionAction::AllowForSession; break;
        case 2: response_action_ = PermissionAction::Deny; break;
      }
      return false;  // Close dialog
    case 27:  // ESC
      has_response_ = true;
      response_action_ = PermissionAction::Deny;
      return false;  // Close dialog
  }
  return true;  // Keep dialog open
}

std::unique_ptr<PermissionResponse> PermissionDialog::get_response() const {
  if (!has_response_) return nullptr;
  
  return std::make_unique<PermissionResponse>(PermissionResponse{
    .permission = permission_,
    .action = response_action_
  });
}

bool PermissionDialog::has_response() const {
  return has_response_;
}

}  // namespace tui