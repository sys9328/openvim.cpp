#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "tool.hpp"

namespace tools {

class GlobTool : public BaseTool {
 public:
  explicit GlobTool(const std::string& working_dir);
  
  std::string name() const override { return "glob"; }
  std::string description() const override { 
    return "Find files by name patterns (glob). Returns matching file paths sorted by modification time.";
  }
  
  ToolResult execute(const std::vector<std::string>& args) override;

 private:
  std::string working_dir_;
  
  std::vector<std::string> glob_files(const std::string& pattern, const std::string& search_path, int limit = 1000);
  bool matches_pattern(const std::string& filename, const std::string& pattern);
};

}  // namespace tools