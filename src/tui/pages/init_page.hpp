#pragma once

#include "tui/page.hpp"

namespace tui {

class InitPage final : public Page {
 public:
  PageId id() const override { return PageId::Init; }
  void on_resize(RenderCtx) override {}
  void on_key(int) override {}
  void render(RenderCtx ctx) override;
};

}  // namespace tui
