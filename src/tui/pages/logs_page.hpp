#pragma once

#include <mutex>
#include <vector>

#include "logging/logger.hpp"
#include "tui/page.hpp"

namespace tui {

class LogsPage final : public Page {
 public:
  explicit LogsPage(logging::Logger& logger);

  PageId id() const override { return PageId::Logs; }
  void on_resize(RenderCtx) override {}
  void on_key(int) override {}
  void render(RenderCtx ctx) override;

 private:
  logging::Logger& logger_;
};

}  // namespace tui
