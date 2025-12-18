#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace pubsub {

enum class EventType {
  Created,
  Updated,
  Deleted,
};

template <typename T>
struct Event {
  EventType type;
  T payload;
};

// Very small thread-safe queue used per-subscriber.
template <typename T>
class Channel {
 public:
  void push(T v) {
    {
      std::lock_guard<std::mutex> lk(mu_);
      q_.push_back(std::move(v));
    }
    cv_.notify_one();
  }

  // Blocking pop; returns std::nullopt if closed.
  std::optional<T> pop() {
    std::unique_lock<std::mutex> lk(mu_);
    cv_.wait(lk, [&] { return closed_ || !q_.empty(); });
    if (q_.empty()) return std::nullopt;
    T v = std::move(q_.front());
    q_.pop_front();
    return v;
  }

  void close() {
    {
      std::lock_guard<std::mutex> lk(mu_);
      closed_ = true;
    }
    cv_.notify_all();
  }

 private:
  std::mutex mu_;
  std::condition_variable cv_;
  std::deque<T> q_;
  bool closed_ = false;
};

// Broker allows clients to publish events and subscribe to events.
// This mirrors the Go design, but returns a shared Channel instead of a Go channel.
template <typename T>
class Broker {
 public:
  using EventT = Event<T>;
  using ChannelT = Channel<EventT>;

  std::shared_ptr<ChannelT> subscribe() {
    std::lock_guard<std::mutex> lk(mu_);
    if (shutdown_) {
      auto ch = std::make_shared<ChannelT>();
      ch->close();
      return ch;
    }
    auto ch = std::make_shared<ChannelT>();
    subs_[next_id_++] = ch;
    return ch;
  }

  void shutdown() {
    std::unordered_map<std::size_t, std::shared_ptr<ChannelT>> subs_copy;
    {
      std::lock_guard<std::mutex> lk(mu_);
      shutdown_ = true;
      subs_copy = subs_;
      subs_.clear();
    }
    for (auto& [_, ch] : subs_copy) {
      ch->close();
    }
  }

  void publish(EventType type, T payload) {
    std::unordered_map<std::size_t, std::shared_ptr<ChannelT>> subs_copy;
    {
      std::lock_guard<std::mutex> lk(mu_);
      if (shutdown_) return;
      subs_copy = subs_;
    }
    EventT ev{type, std::move(payload)};
    for (auto& [_, ch] : subs_copy) {
      ch->push(ev);
    }
  }

 private:
  std::mutex mu_;
  bool shutdown_ = false;
  std::size_t next_id_ = 1;
  std::unordered_map<std::size_t, std::shared_ptr<ChannelT>> subs_;
};

}  // namespace pubsub
