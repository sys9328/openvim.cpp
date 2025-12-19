#include "edit_tool.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace tools {

EditTool::EditTool(const std::string& working_dir) : working_dir_(working_dir) {}

ToolResult EditTool::execute(const std::vector<std::string>& args) {
  if (args.size() < 3) {
    return ToolResult{"file_path, old_string, and new_string are required", true};
  }
  
  std::string file_path = args[0];
  std::string old_string = args[1];
  std::string new_string = args[2];
  
  try {
    if (replace_in_file(file_path, old_string, new_string)) {
      return ToolResult{"File edited successfully: " + file_path};
    } else {
      return ToolResult{"Old string not found in file: " + file_path, true};
    }
  } catch (const std::exception& e) {
    return ToolResult{"Error: " + std::string(e.what()), true};
  }
}

bool EditTool::replace_in_file(const std::string& file_path, const std::string& old_string, const std::string& new_string) {
  std::filesystem::path full_path(file_path);
  if (!std::filesystem::exists(full_path)) {
    throw std::runtime_error("File not found: " + file_path);
  }
  
  if (std::filesystem::is_directory(full_path)) {
    throw std::runtime_error("Path is a directory, not a file: " + file_path);
  }
  
  // Read the entire file
  std::ifstream in_file(full_path, std::ios::binary);
  if (!in_file.is_open()) {
    throw std::runtime_error("Failed to open file for reading: " + file_path);
  }
  
  std::stringstream buffer;
  buffer << in_file.rdbuf();
  std::string content = buffer.str();
  in_file.close();
  
  // Find and replace
  size_t pos = content.find(old_string);
  if (pos == std::string::npos) {
    return false;
  }
  
  content.replace(pos, old_string.length(), new_string);
  
  // Write back to file
  std::ofstream out_file(full_path, std::ios::binary);
  if (!out_file.is_open()) {
    throw std::runtime_error("Failed to open file for writing: " + file_path);
  }
  
  out_file.write(content.c_str(), content.size());
  out_file.close();
  
  return true;
}

}  // namespace tools