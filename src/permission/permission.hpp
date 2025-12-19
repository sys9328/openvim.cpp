#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <thread>
#include <future>

#include "pubsub/broker.hpp"

namespace permission {

struct CreatePermissionRequest {
  std::string tool_name;
  std::string description;
  std::string action;
  std::string path;
  // Note: simplified params as string for now
};

struct PermissionRequest {
  std::string id;
  std::string session_id;
  std::string tool_name;
  std::string description;
  std::string action;
  std::string path;
};

enum class PermissionAction {
  Allow,
  AllowForSession,
  Deny
};

struct PermissionResponse {
  PermissionRequest permission;
  PermissionAction action;
};

class Service {
 public:
  Service();
  
  // Service interface
  std::shared_ptr<pubsub::Channel<pubsub::Event<PermissionRequest>>> subscribe();
  
  void grant_persistent(const PermissionRequest& permission);
  void grant(const PermissionRequest& permission);
  void deny(const PermissionRequest& permission);
  void respond(const PermissionResponse& response);
  
  // Request permission - blocks until user responds or timeout
  bool request(const CreatePermissionRequest& opts);

 private:
  pubsub::Broker<PermissionRequest> broker_;
  std::vector<PermissionRequest> session_permissions_;
  std::unordered_map<std::string, std::shared_ptr<std::promise<bool>>> pending_requests_;
  std::mutex mutex_;
  
  static std::string generate_id();
};

// Global default instance
extern std::unique_ptr<Service> default_service;

}  // namespace permission