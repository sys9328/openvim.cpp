#pragma once

#include <string>
#include <vector>

#include "tool.hpp"

namespace tools {

class LsTool : public BaseTool {
 public:
  explicit LsTool(const std::string& working_dir);
  
  std::string name() const override { return "ls"; }
  std::string description() const override { 
    return "List files and directories in a given path. Shows hierarchical view of directory contents.";
  }
  
  ToolResult execute(const std::vector<std::string>& args) override;

 private:
  std::string working_dir_;
  
  std::string list_directory(const std::string& path, int max_files = 1000);
  bool should_skip(const std::string& path);
};

}  // namespace tools