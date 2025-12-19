#pragma once

#include <string>
#include <vector>

#include "tool.hpp"

namespace tools {

class FileTool : public BaseTool {
 public:
  explicit FileTool(const std::string& working_dir);
  
  std::string name() const override { return "file"; }
  std::string description() const override { 
    return "Basic file operations: check existence, get size, delete files.";
  }
  
  ToolResult execute(const std::vector<std::string>& args) override;

 private:
  std::string working_dir_;
};

}  // namespace tools