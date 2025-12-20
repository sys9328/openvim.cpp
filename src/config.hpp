#pragma once

#include <string>
#include <vector>
#include <map>

namespace config {

enum class MCPType {
  Stdio,
  SSE
};

struct MCPServer {
  std::string name;
  MCPType type;
  std::string command;
  std::vector<std::string> args;
  std::vector<std::string> env;
  std::string url;
  std::map<std::string, std::string> headers;
};

struct Config {
  bool debug = false;
  bool test_mode = false;
  std::string data_dir = ".openvim";
  std::vector<MCPServer> mcp_servers;
  
  // LLM configuration
  std::string llm_api_key;
  std::string llm_base_url = "https://api.openai.com/v1";
  std::string llm_model = "gpt-4";
  int llm_max_tokens = 4096;
  double llm_temperature = 0.7;
};

// Parses argv for commit-4-equivalent flags.
// Supports: -h/--help, -d/--debug, --data-dir <path>
Config parse_args_or_exit(int argc, char** argv);

}  // namespace config
