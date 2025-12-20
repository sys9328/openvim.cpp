#pragma once

#include "tool.hpp"
#include "../config.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <mcp_client.h>
#include <mcp_stdio_client.h>
#include <mcp_sse_client.h>
#include <mcp_tool.h>

namespace tools {

class MCPTool : public BaseTool {
 public:
  MCPTool(const std::string& name, const config::MCPServer& server_config);
  ~MCPTool() override;

  std::string name() const override;
  std::string description() const override;
  ToolResult execute(const std::vector<std::string>& args) override;

  // Get list of available tools from this MCP server
  std::vector<std::string> available_tools() const;

 private:
  void discover_tools();

  std::string name_;
  config::MCPServer server_config_;
  std::unique_ptr<mcp::client> client_;
  std::unordered_map<std::string, mcp::tool> mcp_tools_;
};

}  // namespace tools