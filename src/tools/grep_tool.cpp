#include "grep_tool.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

namespace tools {

GrepTool::GrepTool(const std::string& working_dir) : working_dir_(working_dir) {}

ToolResult GrepTool::execute(const std::vector<std::string>& args) {
  if (args.size() < 1) {
    return ToolResult{"pattern is required", true};
  }
  
  std::string pattern = args[0];
  std::string path = working_dir_;
  std::string include;
  
  if (args.size() >= 2) {
    path = args[1];
  }
  if (args.size() >= 3) {
    include = args[2];
  }
  
  try {
    std::regex regex_pattern(pattern);
    auto matches = grep_files(pattern, path, include);
    
    if (matches.empty()) {
      return ToolResult{"No files found containing pattern: " + pattern};
    }
    
    std::string result = "Found " + std::to_string(matches.size()) + " file(s):\n";
    for (const auto& match : matches) {
      result += match + "\n";
    }
    
    return ToolResult{result};
  } catch (const std::regex_error& e) {
    return ToolResult{"Invalid regex pattern: " + std::string(e.what()), true};
  } catch (const std::exception& e) {
    return ToolResult{"Error: " + std::string(e.what()), true};
  }
}

std::vector<std::string> GrepTool::grep_files(const std::string& pattern, const std::string& path, const std::string& include) {
  std::vector<std::pair<std::string, std::filesystem::file_time_type>> matches;
  
  try {
    std::regex regex_pattern(pattern);
    std::regex include_pattern;
    bool has_include = !include.empty();
    
    if (has_include) {
      std::string include_regex = include;
      include_regex = std::regex_replace(include_regex, std::regex("\\."), "\\.");
      include_regex = std::regex_replace(include_regex, std::regex("\\*"), ".*");
      include_regex = "^" + include_regex + "$";
      include_pattern = std::regex(include_regex);
    }
    
    std::filesystem::path search_path(path);
    if (!std::filesystem::exists(search_path)) {
      return {};
    }
    
    for (const auto& entry : std::filesystem::recursive_directory_iterator(search_path)) {
      if (!entry.is_regular_file()) continue;
      
      std::string file_path = entry.path().string();
      
      if (has_include && !std::regex_match(file_path, include_pattern)) {
        continue;
      }
      
      if (file_contains_pattern(file_path, regex_pattern)) {
        matches.emplace_back(std::filesystem::relative(entry.path(), working_dir_).string(), 
                           entry.last_write_time());
      }
    }
    
    // Sort by modification time (newest first)
    std::sort(matches.begin(), matches.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<std::string> result;
    result.reserve(matches.size());
    for (const auto& match : matches) {
      result.push_back(match.first);
    }
    
    return result;
  } catch (const std::filesystem::filesystem_error&) {
    return {};
  }
}

bool GrepTool::file_contains_pattern(const std::string& file_path, const std::regex& pattern) {
  try {
    std::ifstream file(file_path);
    if (!file.is_open()) return false;
    
    std::string line;
    while (std::getline(file, line)) {
      if (std::regex_search(line, pattern)) {
        return true;
      }
    }
  } catch (const std::exception&) {
    // Skip files we can't read
  }
  return false;
}

}  // namespace tools