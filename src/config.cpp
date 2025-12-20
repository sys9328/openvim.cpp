#include "config.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace config {

static void print_help(std::string_view prog) {
  std::cout << "openvim - a claude code alternative with no vendor lock-in\n\n";
  std::cout << "Usage:\n  " << prog << " [flags]\n\n";
  std::cout << "Flags:\n";
  std::cout << "  -h, --help            Show help\n";
  std::cout << "  -d, --debug           Enable debug logging\n";
  std::cout << "      --test             Test mode (initialize GUI without showing)\n";
  std::cout << "      --data-dir <dir>  Data directory (default: .openvim)\n";
  std::cout << "      --llm-api-key <key> LLM API key (can also set OPENAI_API_KEY env var)\n";
  std::cout << "      --llm-base-url <url> LLM base URL (default: https://api.openai.com/v1)\n";
  std::cout << "      --llm-model <model> LLM model (default: gpt-4)\n";
}

Config parse_args_or_exit(int argc, char** argv) {
  Config cfg;
  std::string_view prog = (argc > 0 && argv && argv[0]) ? argv[0] : "openvim";

  for (int i = 1; i < argc; i++) {
    std::string_view arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      print_help(prog);
      std::exit(0);
    }

    if (arg == "-d" || arg == "--debug") {
      cfg.debug = true;
      continue;
    }

    if (arg == "--test") {
      cfg.test_mode = true;
      continue;
    }

    if (arg == "--data-dir") {
      if (i + 1 >= argc) {
        std::cerr << "--data-dir requires a value\n";
        std::exit(2);
      }
      cfg.data_dir = argv[++i];
      continue;
    }

    if (arg == "--llm-api-key") {
      if (i + 1 >= argc) {
        std::cerr << "--llm-api-key requires a value\n";
        std::exit(2);
      }
      cfg.llm_api_key = argv[++i];
      continue;
    }

    if (arg == "--llm-base-url") {
      if (i + 1 >= argc) {
        std::cerr << "--llm-base-url requires a value\n";
        std::exit(2);
      }
      cfg.llm_base_url = argv[++i];
      continue;
    }

    if (arg == "--llm-model") {
      if (i + 1 >= argc) {
        std::cerr << "--llm-model requires a value\n";
        std::exit(2);
      }
      cfg.llm_model = argv[++i];
      continue;
    }

    std::cerr << "Unknown argument: " << arg << "\n";
    print_help(prog);
    std::exit(2);
  }

  // Check environment variables if not set via command line
  if (cfg.llm_api_key.empty()) {
    const char* env_key = std::getenv("OPENAI_API_KEY");
    if (env_key) {
      cfg.llm_api_key = env_key;
    }
  }

  return cfg;
}

}  // namespace config
