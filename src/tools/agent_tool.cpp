#include "tools/agent_tool.hpp"

namespace tools {

AgentTool::AgentTool(std::string working_dir) : working_dir_(std::move(working_dir)) {}

std::string AgentTool::name() const {
  return "agent";
}

std::string AgentTool::description() const {
  return "Launch a sub-agent for tasks";
}

ToolResult AgentTool::execute(const std::vector<std::string>& args) {
  if (args.empty()) {
    return {"No prompt provided", false};
  }
  // Stub: Simulate sub-agent response
  std::string prompt = args[0];
  std::string response = "Sub-agent result for: " + prompt;
  return {response, true};
}

}  // namespace tools