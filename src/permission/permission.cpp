#include "permission.hpp"

#include <random>
#include <sstream>
#include <iomanip>
#include <future>

namespace permission {

Service::Service() : broker_() {}

std::shared_ptr<pubsub::Channel<pubsub::Event<PermissionRequest>>> Service::subscribe() {
  return broker_.subscribe();
}

void Service::grant_persistent(const PermissionRequest& permission) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Signal the waiting request
  auto it = pending_requests_.find(permission.id);
  if (it != pending_requests_.end()) {
    it->second->set_value(true);
    pending_requests_.erase(it);
  }
  
  // Add to session permissions
  session_permissions_.push_back(permission);
}

void Service::grant(const PermissionRequest& permission) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Signal the waiting request
  auto it = pending_requests_.find(permission.id);
  if (it != pending_requests_.end()) {
    it->second->set_value(true);
    pending_requests_.erase(it);
  }
}

void Service::deny(const PermissionRequest& permission) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Signal the waiting request with false
  auto it = pending_requests_.find(permission.id);
  if (it != pending_requests_.end()) {
    it->second->set_value(false);
    pending_requests_.erase(it);
  }
}

void Service::respond(const PermissionResponse& response) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  switch (response.action) {
    case PermissionAction::Allow:
      grant(response.permission);
      break;
    case PermissionAction::AllowForSession:
      grant_persistent(response.permission);
      break;
    case PermissionAction::Deny:
      deny(response.permission);
      break;
  }
}

bool Service::request(const CreatePermissionRequest& opts) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Check if already granted for this session
  for (const auto& p : session_permissions_) {
    if (p.tool_name == opts.tool_name && p.action == opts.action) {
      return true;
    }
  }
  
  // Create permission request
  PermissionRequest permission{
    .id = generate_id(),
    .tool_name = opts.tool_name,
    .description = opts.description,
    .action = opts.action,
    .path = opts.path
  };
  
  // Create promise for the response
  auto promise = std::make_shared<std::promise<bool>>();
  auto future = promise->get_future();
  pending_requests_[permission.id] = promise;
  
  // Publish the permission request
  broker_.publish(pubsub::EventType::Created, permission);
  
  // Wait for response with timeout (10 minutes)
  auto status = future.wait_for(std::chrono::minutes(10));
  if (status == std::future_status::timeout) {
    // Remove from pending requests
    pending_requests_.erase(permission.id);
    return false;
  }
  
  return future.get();
}

std::string Service::generate_id() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, 15);
  
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  
  for (int i = 0; i < 32; ++i) {
    ss << dis(gen);
  }
  
  return ss.str();
}

// Global default instance
std::unique_ptr<Service> default_service = std::make_unique<Service>();

}  // namespace permission