#include "write_tool.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include "../permission/permission.hpp"

namespace tools {

WriteTool::WriteTool(const std::string& working_dir) : working_dir_(working_dir) {}

ToolResult WriteTool::execute(const std::vector<std::string>& args) {
  if (args.size() < 2) {
    return ToolResult{"file_path and content are required", true};
  }
  
  std::string file_path = args[0];
  std::string content = args[1];
  
  // Request permission
  permission::CreatePermissionRequest perm_req{
    .tool_name = "write",
    .description = "Write to file " + file_path,
    .action = "write",
    .path = working_dir_
  };
  
  if (!permission::default_service->request(perm_req)) {
    return ToolResult{"Permission denied", true};
  }
  
  try {
    std::filesystem::path full_path(file_path);
    
    // Create parent directories if they don't exist
    std::filesystem::create_directories(full_path.parent_path());
    
    // Check if file exists and is not a directory
    if (std::filesystem::exists(full_path) && std::filesystem::is_directory(full_path)) {
      return ToolResult{"Path is a directory, not a file: " + file_path, true};
    }
    
    // Write the file
    std::ofstream file(full_path, std::ios::binary);
    if (!file.is_open()) {
      return ToolResult{"Failed to open file for writing: " + file_path, true};
    }
    
    file.write(content.c_str(), content.size());
    file.close();
    
    return ToolResult{"File written: " + file_path};
  } catch (const std::exception& e) {
    return ToolResult{"Error: " + std::string(e.what()), true};
  }
}

}  // namespace tools