#pragma once

#include <string>
#include <vector>

namespace tools {

struct ToolResult {
  std::string output;
  bool success;
};

class BaseTool {
 public:
  virtual ~BaseTool() = default;
  virtual std::string name() const = 0;
  virtual std::string description() const = 0;
  virtual ToolResult execute(const std::vector<std::string>& args) = 0;
};

}  // namespace tools