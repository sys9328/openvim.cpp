#include "mcp_tool.hpp"

#include <memory>
#include <stdexcept>
#include <sstream>
#include <iostream>

#include "../permission/permission.hpp"

namespace tools {

MCPTool::MCPTool(const std::string& name, const config::MCPServer& server_config)
    : name_(name), server_config_(server_config) {
  
  // Initialize the appropriate MCP client based on server type
  if (server_config_.type == config::MCPType::Stdio) {
    // Convert env vars to JSON
    nlohmann::json env_json = nlohmann::json::object();
    for (const auto& env_var : server_config_.env) {
      // Simple parsing of KEY=VALUE format
      size_t eq_pos = env_var.find('=');
      if (eq_pos != std::string::npos) {
        std::string key = env_var.substr(0, eq_pos);
        std::string value = env_var.substr(eq_pos + 1);
        env_json[key] = value;
      }
    }
    
    client_ = std::make_unique<mcp::stdio_client>(server_config_.command, env_json);
    
  } else if (server_config_.type == config::MCPType::SSE) {
    client_ = std::make_unique<mcp::sse_client>(server_config_.url);
  }
  
  // Initialize the client
  if (!client_->initialize("openvim", "1.0.0")) {
    throw std::runtime_error("Failed to initialize MCP client for server: " + name);
  }
  
  // Discover available tools
  discover_tools();
}

MCPTool::~MCPTool() = default;

void MCPTool::discover_tools() {
  try {
    auto tools_list = client_->get_tools();
    mcp_tools_.clear();
    
    for (const auto& tool : tools_list) {
      mcp_tools_[tool.name] = tool;
    }
  } catch (const std::exception& e) {
    std::cerr << "Failed to discover tools for MCP server " << name_ << ": " << e.what() << std::endl;
  }
}

std::string MCPTool::name() const {
  return name_;
}

std::string MCPTool::description() const {
  return "MCP server providing various tools";
}

ToolResult MCPTool::execute(const std::vector<std::string>& args) {
  ToolResult result;
  
  if (args.empty()) {
    result.success = false;
    result.output = "Usage: " + name_ + " <tool_name> [args...]";
    return result;
  }
  
  std::string tool_name = args[0];
  
  // Check if the requested tool exists
  auto it = mcp_tools_.find(tool_name);
  if (it == mcp_tools_.end()) {
    result.success = false;
    result.output = "Tool '" + tool_name + "' not found in MCP server '" + name_ + "'";
    return result;
  }
  
  // Request permission for executing the MCP tool
  std::string param_str;
  if (args.size() > 1) {
    // Join remaining args as JSON string
    for (size_t i = 1; i < args.size(); ++i) {
      if (i > 1) param_str += " ";
      param_str += args[i];
    }
  } else {
    param_str = "{}";  // Empty JSON object
  }
  
  permission::CreatePermissionRequest perm_request;
  perm_request.tool_name = name_ + "." + tool_name;
  perm_request.action = "execute";
  perm_request.description = "Execute MCP tool '" + tool_name + "' with parameters: " + param_str;
  perm_request.path = "";
  
  bool permission_result = permission::default_service->request(perm_request);
  
  if (!permission_result) {
    result.success = false;
    result.output = "Permission denied for executing MCP tool '" + tool_name + "'";
    return result;
  }
  
  try {
    // Parse parameters as JSON
    nlohmann::json params;
    if (!param_str.empty() && param_str != "{}") {
      params = nlohmann::json::parse(param_str);
    } else {
      params = nlohmann::json::object();
    }
    
    // Call the MCP tool
    auto result_json = client_->call_tool(tool_name, params);
    
    // Format the result
    std::string output;
    if (result_json.is_array()) {
      for (const auto& item : result_json) {
        if (item.contains("text")) {
          output += item["text"].get<std::string>();
        } else {
          output += item.dump(2);
        }
        output += "\n";
      }
    } else {
      output = result_json.dump(2);
    }
    
    result.success = true;
    result.output = output;
    return result;
    
  } catch (const std::exception& e) {
    result.success = false;
    result.output = "Error executing MCP tool '" + tool_name + "': " + e.what();
    return result;
  }
}

std::vector<std::string> MCPTool::available_tools() const {
  std::vector<std::string> tools;
  for (const auto& [name, _] : mcp_tools_) {
    tools.push_back(name);
  }
  return tools;
}

}  // namespace tools