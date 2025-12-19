#include "file_tool.hpp"

#include <filesystem>
#include <iostream>

namespace tools {

FileTool::FileTool(const std::string& working_dir) : working_dir_(working_dir) {}

ToolResult FileTool::execute(const std::vector<std::string>& args) {
  if (args.empty()) {
    return ToolResult{"operation is required (read, exists, size, delete)", true};
  }
  
  std::string operation = args[0];
  
  if (operation == "exists") {
    if (args.size() < 2) {
      return ToolResult{"file_path is required for exists operation", true};
    }
    std::string file_path = args[1];
    bool exists = std::filesystem::exists(file_path);
    return ToolResult{exists ? "true" : "false"};
  }
  
  if (operation == "size") {
    if (args.size() < 2) {
      return ToolResult{"file_path is required for size operation", true};
    }
    std::string file_path = args[1];
    try {
      auto size = std::filesystem::file_size(file_path);
      return ToolResult{std::to_string(size)};
    } catch (const std::exception& e) {
      return ToolResult{"Error: " + std::string(e.what()), true};
    }
  }
  
  if (operation == "delete") {
    if (args.size() < 2) {
      return ToolResult{"file_path is required for delete operation", true};
    }
    std::string file_path = args[1];
    try {
      if (std::filesystem::remove(file_path)) {
        return ToolResult{"File deleted: " + file_path};
      } else {
        return ToolResult{"File not found: " + file_path, true};
      }
    } catch (const std::exception& e) {
      return ToolResult{"Error: " + std::string(e.what()), true};
    }
  }
  
  return ToolResult{"Unknown operation: " + operation + ". Supported: exists, size, delete", true};
}

}  // namespace tools