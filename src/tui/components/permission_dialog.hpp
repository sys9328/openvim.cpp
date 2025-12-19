#pragma once

#include <string>
#include <vector>
#include <memory>

#include "permission/permission.hpp"
#include "tui/styles.hpp"

namespace tui {

enum class PermissionAction {
  Allow,
  AllowForSession,
  Deny
};

struct PermissionResponse {
  permission::PermissionRequest permission;
  PermissionAction action;
};

class PermissionDialog {
 public:
  PermissionDialog(const permission::PermissionRequest& request, const std::string& content);
  
  void render(WINDOW* win, int width, int height);
  bool handle_input(int ch);
  
  std::unique_ptr<PermissionResponse> get_response() const;
  bool has_response() const;
  
  int get_height() const { return 8; }
  int get_width() const { return 60; }

 private:
  permission::PermissionRequest permission_;
  std::string content_;
  int selected_option_ = 0;
  bool has_response_ = false;
  PermissionAction response_action_;
  
  std::vector<std::string> options_ = {"Allow", "Allow for this session", "Deny"};
};

}  // namespace tui