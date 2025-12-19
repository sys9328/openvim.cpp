#include "view_tool.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace tools {

ViewTool::ViewTool(const std::string& working_dir) : working_dir_(working_dir) {}

ToolResult ViewTool::execute(const std::vector<std::string>& args) {
  if (args.empty()) {
    return ToolResult{"file_path is required", true};
  }
  
  std::string file_path = args[0];
  int offset = 0;
  int limit = 2000;
  
  if (args.size() >= 2) {
    try {
      offset = std::stoi(args[1]);
    } catch (...) {
      return ToolResult{"Invalid offset parameter", true};
    }
  }
  
  if (args.size() >= 3) {
    try {
      limit = std::stoi(args[2]);
    } catch (...) {
      return ToolResult{"Invalid limit parameter", true};
    }
  }
  
  try {
    if (is_image_file(file_path)) {
      return ToolResult{"Reading images is not supported yet"};
    }
    
    std::string content = read_file(file_path, offset, limit);
    return ToolResult{content};
  } catch (const std::exception& e) {
    return ToolResult{"Error: " + std::string(e.what()), true};
  }
}

std::string ViewTool::read_file(const std::string& file_path, int offset, int limit) {
  std::filesystem::path full_path(file_path);
  if (!std::filesystem::exists(full_path)) {
    return "File not found: " + file_path;
  }
  
  if (std::filesystem::is_directory(full_path)) {
    return "Path is a directory, not a file: " + file_path;
  }
  
  // Check file size (250KB limit)
  auto file_size = std::filesystem::file_size(full_path);
  if (file_size > 250 * 1024) {
    return "File is too large (" + std::to_string(file_size) + " bytes). Maximum size is 250KB";
  }
  
  std::ifstream file(full_path);
  if (!file.is_open()) {
    return "Failed to open file: " + file_path;
  }
  
  std::vector<std::string> lines;
  std::string line;
  
  // Skip to offset
  int current_line = 0;
  while (current_line < offset && std::getline(file, line)) {
    current_line++;
  }
  
  // Read lines up to limit
  int lines_read = 0;
  while (lines_read < limit && std::getline(file, line)) {
    // Truncate long lines
    if (line.length() > 2000) {
      line = line.substr(0, 2000) + "...";
    }
    lines.push_back(line);
    lines_read++;
  }
  
  std::string content;
  for (size_t i = 0; i < lines.size(); ++i) {
    if (i > 0) content += "\n";
    content += lines[i];
  }
  
  return add_line_numbers(content, offset + 1);
}

std::string ViewTool::add_line_numbers(const std::string& content, int start_line) {
  if (content.empty()) {
    return "";
  }
  
  std::vector<std::string> lines;
  std::stringstream ss(content);
  std::string line;
  
  while (std::getline(ss, line)) {
    lines.push_back(line);
  }
  
  std::stringstream result;
  for (size_t i = 0; i < lines.size(); ++i) {
    int line_num = start_line + static_cast<int>(i);
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%6d", line_num);
    result << buffer << "\t|" << lines[i] << "\n";
  }
  
  return result.str();
}

bool ViewTool::is_image_file(const std::string& file_path) {
  std::string ext = std::filesystem::path(file_path).extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  
  return ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".gif" || 
         ext == ".bmp" || ext == ".svg" || ext == ".webp";
}

}  // namespace tools