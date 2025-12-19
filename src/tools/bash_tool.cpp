#include "tools/bash_tool.hpp"

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace tools {

BashTool::BashTool(std::string working_dir) : working_dir_(std::move(working_dir)) {}

std::string BashTool::name() const {
  return "bash";
}

std::string BashTool::description() const {
  return "Execute bash commands";
}

ToolResult BashTool::execute(const std::vector<std::string>& args) {
  if (args.empty()) {
    return {"No command provided", false};
  }

  std::string cmd = "cd " + working_dir_ + " && " + args[0];
  for (size_t i = 1; i < args.size(); ++i) {
    cmd += " " + args[i];
  }

  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe) {
    return {"Failed to run command", false};
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return {result, true};
}

}  // namespace tools