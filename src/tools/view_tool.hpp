#pragma once

#include <string>
#include <vector>

#include "tool.hpp"

namespace tools {

class ViewTool : public BaseTool {
 public:
  explicit ViewTool(const std::string& working_dir);
  
  std::string name() const override { return "view"; }
  std::string description() const override { 
    return "Read and display file contents with line numbers. Supports offset and limit for large files.";
  }
  
  ToolResult execute(const std::vector<std::string>& args) override;

 private:
  std::string working_dir_;
  
  std::string read_file(const std::string& file_path, int offset = 0, int limit = 2000);
  std::string add_line_numbers(const std::string& content, int start_line);
  bool is_image_file(const std::string& file_path);
};

}  // namespace tools