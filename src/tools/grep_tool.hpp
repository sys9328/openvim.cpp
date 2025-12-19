#pragma once

#include <string>
#include <vector>
#include <regex>

#include "tool.hpp"

namespace tools {

class GrepTool : public BaseTool {
 public:
  explicit GrepTool(const std::string& working_dir);
  
  std::string name() const override { return "grep"; }
  std::string description() const override { 
    return "Search file contents using regular expressions. Finds files containing specific patterns.";
  }
  
  ToolResult execute(const std::vector<std::string>& args) override;

 private:
  std::string working_dir_;
  
  std::vector<std::string> grep_files(const std::string& pattern, const std::string& path, const std::string& include = "");
  bool file_contains_pattern(const std::string& file_path, const std::regex& pattern);
};

}  // namespace tools