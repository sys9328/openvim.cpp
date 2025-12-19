#include "ls_tool.hpp"

#include <filesystem>
#include <iostream>
#include <sstream>

namespace tools {

LsTool::LsTool(const std::string& working_dir) : working_dir_(working_dir) {}

ToolResult LsTool::execute(const std::vector<std::string>& args) {
  if (args.empty()) {
    return ToolResult{"path is required", true};
  }
  
  std::string path = args[0];
  
  try {
    std::string result = list_directory(path);
    return ToolResult{result};
  } catch (const std::exception& e) {
    return ToolResult{"Error: " + std::string(e.what()), true};
  }
}

std::string LsTool::list_directory(const std::string& path, int max_files) {
  std::filesystem::path dir_path(path);
  if (!std::filesystem::exists(dir_path) || !std::filesystem::is_directory(dir_path)) {
    return "Path does not exist or is not a directory: " + path;
  }
  
  std::vector<std::string> files;
  int count = 0;
  
  try {
    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
      if (count >= max_files) break;
      
      std::string entry_path = entry.path().string();
      if (should_skip(entry_path)) continue;
      
      std::string relative_path = std::filesystem::relative(entry.path(), working_dir_).string();
      if (entry.is_directory()) {
        relative_path += "/";
      }
      files.push_back(relative_path);
      count++;
    }
  } catch (const std::filesystem::filesystem_error& e) {
    return "Error accessing directory: " + std::string(e.what());
  }
  
  if (files.empty()) {
    return "Directory is empty";
  }
  
  std::stringstream result;
  result << "- " << std::filesystem::relative(dir_path, working_dir_).string() << "/\n";
  
  for (const auto& file : files) {
    result << "  - " << std::filesystem::path(file).filename().string();
    if (file.back() == '/') {
      result << "/";
    }
    result << "\n";
  }
  
  if (count >= max_files) {
    result << "\n(Results are truncated. There are more than " << max_files << " files.)";
  }
  
  return result.str();
}

bool LsTool::should_skip(const std::string& path) {
  std::string filename = std::filesystem::path(path).filename().string();
  
  // Skip hidden files/directories
  if (!filename.empty() && filename[0] == '.') {
    return true;
  }
  
  // Skip common system directories
  if (filename == "__pycache__") {
    return true;
  }
  
  return false;
}

}  // namespace tools