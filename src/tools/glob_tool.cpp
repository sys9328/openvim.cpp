#include "glob_tool.hpp"

#include <algorithm>
#include <iostream>
#include <regex>

namespace tools {

GlobTool::GlobTool(const std::string& working_dir) : working_dir_(working_dir) {}

ToolResult GlobTool::execute(const std::vector<std::string>& args) {
  if (args.size() < 1) {
    return ToolResult{"pattern is required", true};
  }
  
  std::string pattern = args[0];
  std::string path = working_dir_;
  
  if (args.size() >= 2) {
    path = args[1];
  }
  
  try {
    auto matches = glob_files(pattern, path);
    
    if (matches.empty()) {
      return ToolResult{"No files found matching pattern: " + pattern};
    }
    
    std::string result = "Found " + std::to_string(matches.size()) + " file(s):\n";
    for (const auto& match : matches) {
      result += match + "\n";
    }
    
    return ToolResult{result};
  } catch (const std::exception& e) {
    return ToolResult{"Error: " + std::string(e.what()), true};
  }
}

std::vector<std::string> GlobTool::glob_files(const std::string& pattern, const std::string& search_path, int limit) {
  std::vector<std::string> results;
  
  try {
    std::filesystem::path search_dir(search_path);
    if (!std::filesystem::exists(search_dir) || !std::filesystem::is_directory(search_dir)) {
      return results;
    }
    
    for (const auto& entry : std::filesystem::recursive_directory_iterator(search_dir)) {
      if (results.size() >= limit) break;
      
      std::string filename = entry.path().filename().string();
      if (matches_pattern(filename, pattern)) {
        results.push_back(std::filesystem::relative(entry.path(), working_dir_).string());
      }
    }
    
    // Sort by modification time (newest first)
    std::sort(results.begin(), results.end(), [&](const std::string& a, const std::string& b) {
      auto path_a = std::filesystem::path(working_dir_) / a;
      auto path_b = std::filesystem::path(working_dir_) / b;
      auto time_a = std::filesystem::last_write_time(path_a);
      auto time_b = std::filesystem::last_write_time(path_b);
      return time_a > time_b;
    });
    
  } catch (const std::filesystem::filesystem_error& e) {
    // Continue with empty results
  }
  
  return results;
}

bool GlobTool::matches_pattern(const std::string& filename, const std::string& pattern) {
  // Simple glob to regex conversion
  std::string regex_pattern = pattern;
  
  // Escape special regex characters
  regex_pattern = std::regex_replace(regex_pattern, std::regex("\\."), "\\.");
  regex_pattern = std::regex_replace(regex_pattern, std::regex("\\?"), ".");
  regex_pattern = std::regex_replace(regex_pattern, std::regex("\\*"), ".*");
  
  // Handle {a,b,c} patterns
  std::regex brace_regex("\\{([^}]+)\\}");
  std::smatch brace_match;
  if (std::regex_search(regex_pattern, brace_match, brace_regex)) {
    std::string inner = brace_match[1].str();
    std::string replacement = "(" + std::regex_replace(inner, std::regex(","), "|") + ")";
    regex_pattern = std::regex_replace(regex_pattern, brace_regex, replacement);
  }
  
  regex_pattern = "^" + regex_pattern + "$";
  
  try {
    std::regex regex(regex_pattern);
    return std::regex_match(filename, regex);
  } catch (const std::regex_error&) {
    return false;
  }
}

}  // namespace tools