#pragma once

#include <string>
#include <vector>

#include "tool.hpp"

namespace tools {

class WriteTool : public BaseTool {
 public:
  explicit WriteTool(const std::string& working_dir);
  
  std::string name() const override { return "write"; }
  std::string description() const override { 
    return "Write content to files. Creates parent directories if needed. Overwrites existing files.";
  }
  
  ToolResult execute(const std::vector<std::string>& args) override;

 private:
  std::string working_dir_;
};

}  // namespace tools