#pragma once

#include <string>
#include <vector>

#include "tool.hpp"

namespace tools {

class EditTool : public BaseTool {
 public:
  explicit EditTool(const std::string& working_dir);
  
  std::string name() const override { return "edit"; }
  std::string description() const override { 
    return "Edit files by replacing text. Finds and replaces exact string matches in files.";
  }
  
  ToolResult execute(const std::vector<std::string>& args) override;

 private:
  std::string working_dir_;
  
  bool replace_in_file(const std::string& file_path, const std::string& old_string, const std::string& new_string);
};

}  // namespace tools