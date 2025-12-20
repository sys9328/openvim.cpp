#include "llm/llm.hpp"

#include <chrono>
#include <sstream>
#include <iostream>
#include <thread>
#include <future>

#include "httplib.h"
#include "json.hpp"

#include "tools/agent_tool.hpp"
#include "tools/bash_tool.hpp"
#include "tools/edit_tool.hpp"
#include "tools/file_tool.hpp"
#include "tools/glob_tool.hpp"
#include "tools/grep_tool.hpp"
#include "tools/ls_tool.hpp"
#include "tools/view_tool.hpp"
#include "tools/write_tool.hpp"
#include "tools/mcp_tool.hpp"

namespace llm {

Service::Service(logging::Logger& log, message::Service& messages, const config::Config& config) 
    : log_(log), messages_(messages), config_(config) {
  // Initialize tools
  tools_.push_back(std::make_unique<tools::BashTool>(working_dir_));
  tools_.push_back(std::make_unique<tools::AgentTool>(working_dir_));
  tools_.push_back(std::make_unique<tools::EditTool>(working_dir_));
  tools_.push_back(std::make_unique<tools::FileTool>(working_dir_));
  tools_.push_back(std::make_unique<tools::GlobTool>(working_dir_));
  tools_.push_back(std::make_unique<tools::GrepTool>(working_dir_));
  tools_.push_back(std::make_unique<tools::LsTool>(working_dir_));
  tools_.push_back(std::make_unique<tools::ViewTool>(working_dir_));
  tools_.push_back(std::make_unique<tools::WriteTool>(working_dir_));
  
  // Initialize MCP tools
  for (const auto& mcp_server : config_.mcp_servers) {
    try {
      tools_.push_back(std::make_unique<tools::MCPTool>(mcp_server.name, mcp_server));
      log_.info("Initialized MCP tool: " + mcp_server.name);
    } catch (const std::exception& e) {
      log_.error("Failed to initialize MCP tool '" + mcp_server.name + "': " + e.what());
    }
  }
}

void Service::send_request(const std::string& session_id, const std::string& content) {
  broker_.publish(pubsub::EventType::Created, AgentEvent{AgentEventType::Request, session_id, content});
  std::thread(&Service::worker, this, session_id, content).detach();
}

void Service::worker(std::string session_id, std::string content) {
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(300ms);

  try {
    // Get conversation history for this session
    auto conversation_messages = messages_.list(session_id);
    
    // Build messages array for LLM API
    nlohmann::json messages_json = nlohmann::json::array();
    
    // Add system message
    nlohmann::json system_msg = nlohmann::json::object();
    system_msg["role"] = "system";
    system_msg["content"] = "You are an AI assistant with access to various tools. Use the tools when appropriate to help the user. Be concise but helpful.";
    messages_json.push_back(system_msg);
    
    // Add conversation history (excluding system messages)
    for (const auto& msg : conversation_messages) {
      nlohmann::json msg_json = nlohmann::json::object();
      msg_json["role"] = (msg.role == message::Role::User) ? "user" : "assistant";
      msg_json["content"] = msg.content;
      messages_json.push_back(msg_json);
    }
    
    // Add the new user message if not already in history
    bool has_current_message = false;
    for (const auto& msg : conversation_messages) {
      if (msg.content == content && msg.role == message::Role::User) {
        has_current_message = true;
        break;
      }
    }
    if (!has_current_message) {
      nlohmann::json user_msg = nlohmann::json::object();
      user_msg["role"] = "user";
      user_msg["content"] = content;
      messages_json.push_back(user_msg);
    }
    
    // Build tools array for LLM API
    nlohmann::json tools_json = nlohmann::json::array();
    for (const auto& tool : tools_) {
      nlohmann::json tool_def = nlohmann::json::object();
      tool_def["type"] = "function";
      
      nlohmann::json function_def = nlohmann::json::object();
      function_def["name"] = tool->name();
      function_def["description"] = tool->description();
      
      nlohmann::json parameters = nlohmann::json::object();
      parameters["type"] = "object";
      
      nlohmann::json properties = nlohmann::json::object();
      nlohmann::json args_prop = nlohmann::json::object();
      args_prop["type"] = "array";
      args_prop["items"] = nlohmann::json::object({{"type", "string"}});
      args_prop["description"] = "Command line arguments to pass to the tool";
      properties["args"] = args_prop;
      
      parameters["properties"] = properties;
      nlohmann::json required = nlohmann::json::array();
      required.push_back("args");
      parameters["required"] = required;
      
      function_def["parameters"] = parameters;
      tool_def["function"] = function_def;
      
      tools_json.push_back(tool_def);
    }
    
    // Make LLM API call
    std::string response = call_llm_api(session_id, messages_json, tools_json);
    
    // Create assistant message
    messages_.create_assistant(session_id, response);
    broker_.publish(pubsub::EventType::Created, AgentEvent{AgentEventType::Response, session_id, response});
    
  } catch (const std::exception& e) {
    log_.error(std::string("LLM error: ") + e.what());
    std::string error_msg = "Sorry, I encountered an error: " + std::string(e.what());
    messages_.create_assistant(session_id, error_msg);
    broker_.publish(pubsub::EventType::Created,
                    AgentEvent{AgentEventType::Error, session_id, error_msg});
  }
}

std::shared_ptr<pubsub::Channel<pubsub::Event<AgentEvent>>> Service::subscribe() {
  return broker_.subscribe();
}

std::string Service::generate_title(const std::string& content) {
  // Stub: Generate a simple title based on content.
  // In a real implementation, send to LLM with a prompt like:
  // "Generate a short title (max 80 chars) summarizing this message: " + content
  if (content.length() <= 50) {
    return content;
  } else {
    return content.substr(0, 47) + "...";
  }
}

std::string Service::call_llm_api(const std::string& session_id, nlohmann::json& messages, const nlohmann::json& tools) {
  if (config_.llm_api_key.empty()) {
    throw std::runtime_error("LLM API key not configured. Set OPENAI_API_KEY environment variable or use --llm-api-key flag.");
  }
  
  // Create HTTP client
  httplib::Client cli(config_.llm_base_url.c_str());
  
  // Build request headers
  httplib::Headers headers = {
    {"Authorization", "Bearer " + config_.llm_api_key},
    {"Content-Type", "application/json"}
  };
  nlohmann::json payload = {
    {"model", config_.llm_model},
    {"messages", messages},
    {"max_tokens", config_.llm_max_tokens},
    {"temperature", config_.llm_temperature}
  };
  
  if (!tools.empty()) {
    payload["tools"] = tools;
    payload["tool_choice"] = "auto";
  }
  
  // Make request
  auto res = cli.Post("/chat/completions", headers, payload.dump(), "application/json");
  
  if (!res || res->status != 200) {
    std::string error_msg = "LLM API request failed";
    if (res) {
      error_msg += " (status: " + std::to_string(res->status) + ")";
      if (!res->body.empty()) {
        error_msg += ": " + res->body;
      }
    }
    throw std::runtime_error(error_msg);
  }
  
  // Parse response
  nlohmann::json response = nlohmann::json::parse(res->body);
  
  if (!response.contains("choices") || response["choices"].empty()) {
    throw std::runtime_error("Invalid LLM API response: missing choices");
  }
  
  auto& choice = response["choices"][0];
  if (!choice.contains("message")) {
    throw std::runtime_error("Invalid LLM API response: missing message");
  }
  
  auto& message = choice["message"];
  std::string content = message.value("content", "");
  
  // Handle tool calls
  if (message.contains("tool_calls") && !message["tool_calls"].empty()) {
    return handle_tool_calls(session_id, message["tool_calls"], messages);
  }
  
  return content;
}

std::string Service::handle_tool_calls(const std::string& session_id, const nlohmann::json& tool_calls, nlohmann::json& messages) {
  // Add the assistant's message with tool calls to the conversation
  nlohmann::json assistant_message = nlohmann::json::object();
  assistant_message["role"] = "assistant";
  assistant_message["tool_calls"] = tool_calls;
  messages.push_back(assistant_message);
  
  // Execute each tool call
  for (const auto& tool_call : tool_calls) {
    std::string tool_call_id = tool_call["id"];
    std::string function_name = tool_call["function"]["name"];
    nlohmann::json function_args = nlohmann::json::parse(tool_call["function"]["arguments"].get<std::string>());
    
    // Find the tool
    tools::BaseTool* tool = nullptr;
    for (const auto& t : tools_) {
      if (t->name() == function_name) {
        tool = t.get();
        break;
      }
    }
    
    if (!tool) {
      log_.error("Tool not found: " + function_name);
      continue;
    }
    
    // Extract arguments
    std::vector<std::string> args;
    if (function_args.contains("args") && function_args["args"].is_array()) {
      for (const auto& arg : function_args["args"]) {
        args.push_back(arg.get<std::string>());
      }
    }
    
    // Execute tool
    tools::ToolResult result = tool->execute(args);
    
    // Add tool result to messages
    nlohmann::json tool_message = nlohmann::json::object();
    tool_message["role"] = "tool";
    tool_message["tool_call_id"] = tool_call_id;
    tool_message["content"] = result.output;
    messages.push_back(tool_message);
  }
  
  // Make follow-up LLM call with tool results
  nlohmann::json follow_up_tools = nlohmann::json::array(); // Empty tools for follow-up
  return call_llm_api(session_id, messages, follow_up_tools);
}

}  // namespace llm
