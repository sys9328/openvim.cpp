#pragma once

#include "tools/tool.hpp"

namespace tools {

class BashTool : public BaseTool {
 public:
  explicit BashTool(std::string working_dir);

  std::string name() const override;
  std::string description() const override;
  ToolResult execute(const std::vector<std::string>& args) override;

 private:
  std::string working_dir_;
};

}  // namespace tools