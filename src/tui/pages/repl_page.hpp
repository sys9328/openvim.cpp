#pragma once

#include <functional>
#include <string>
#include <vector>

#include "tui/page.hpp"

namespace tui {

class ReplPage final : public Page {
 public:
  using SessionsProvider = std::function<std::vector<std::string>()>;
  using MessagesProvider = std::function<std::vector<std::string>()>;
  using SendFn = std::function<void(std::string)>;

  ReplPage(SessionsProvider sessions_provider, MessagesProvider messages_provider, SendFn send);

  PageId id() const override { return PageId::Repl; }
  void on_resize(RenderCtx) override {}
  void on_key(int ch) override;
  void render(RenderCtx ctx) override;

 private:
  SessionsProvider sessions_provider_;
  MessagesProvider messages_provider_;
  SendFn send_;

  std::string input_;
};

}  // namespace tui
