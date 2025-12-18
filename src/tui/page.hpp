#pragma once

#include <string>

namespace tui {

enum class PageId {
  Init,
  Repl,
  Logs,
};

struct RenderCtx {
  int width = 0;
  int height = 0;
};

class Page {
 public:
  virtual ~Page() = default;
  virtual PageId id() const = 0;
  virtual void on_resize(RenderCtx ctx) = 0;
  virtual void on_key(int ch) = 0;
  virtual void render(RenderCtx ctx) = 0;
};

}  // namespace tui
