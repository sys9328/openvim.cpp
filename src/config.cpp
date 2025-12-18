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
  std::cout << "      --data-dir <dir>  Data directory (default: .openvim)\n";
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

    if (arg == "--data-dir") {
      if (i + 1 >= argc) {
        std::cerr << "--data-dir requires a value\n";
        std::exit(2);
      }
      cfg.data_dir = argv[++i];
      continue;
    }

    std::cerr << "Unknown argument: " << arg << "\n";
    print_help(prog);
    std::exit(2);
  }

  return cfg;
}

}  // namespace config
