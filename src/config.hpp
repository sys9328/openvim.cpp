#pragma once

#include <string>

namespace config {

struct Config {
  bool debug = false;
  std::string data_dir = ".openvim";  
};

// Parses argv for commit-4-equivalent flags.
// Supports: -h/--help, -d/--debug, --data-dir <path>
Config parse_args_or_exit(int argc, char** argv);

}  // namespace config
