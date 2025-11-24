#include "health/HealthServer.hpp"
#include "config/Config.hpp"
#include "logging/Logger.hpp"
#include "replication/ReplicationManager.hpp"
#include <spdlog/spdlog.h>
#include <httplib.h> // Assuming httplib.h is in include/

namespace SyncLayer::Health {

HealthServer::HealthServer(std::shared_ptr<SyncLayer::Config::Config> config,
                           std::shared_ptr<SyncLayer::Logging::Logger> logger,
                           std::unique_ptr<SyncLayer::Replication::ReplicationManager>& replicationManager)
    : config_(std::move(config)), logger_(std::move(logger)), replicationManager_(replicationManager), running_(false) {}

HealthServer::~HealthServer() {
    stop();
}

void HealthServer::start() {
    if (running_) return;
    running_ = true;
    serverThread_ = std::thread(&HealthServer::runServer, this);
}

void HealthServer::stop() {
    if (!running_) return;
    running_ = false;
    if (serverThread_.joinable()) {
        serverThread_.join();
    }
}

void HealthServer::runServer() {
    httplib::Server server;
    server.Get("/health", [this](const httplib::Request&, httplib::Response& res) {
        if (!replicationManager_) {
            std::string json = R"(
{
  "status": "unhealthy",
  "message": "Replication manager not initialized - database connection failed",
  "databases": {
    "local": {
      "status": "unhealthy",
      "connections": 0,
      "size_mb": 0,
      "active_queries": 0
    },
    "hosted": {
      "status": "unhealthy",
      "connections": 0,
      "size_mb": 0,
      "active_queries": 0
    }
  }
}
)";
            res.set_content(json, "application/json");
            res.status = 503;
            return;
        }
        
        auto health = replicationManager_->healthCheck();
        std::string status = health.overall ? "healthy" : "unhealthy";
        std::string localStatus = health.localDb ? "healthy" : "unhealthy";
        std::string hostedStatus = health.hostedDb ? "healthy" : "unhealthy";
        
        std::string json = R"(
{
  "status": ")" + status + R"(",
  "databases": {
    "local": {
      "status": ")" + localStatus + R"(",
      "connections": )" + std::to_string(health.localConnections) + R"(,
      "size_mb": )" + std::to_string(health.localDbSizeMB) + R"(,
      "active_queries": )" + std::to_string(health.localActiveQueries) + R"(
    },
    "hosted": {
      "status": ")" + hostedStatus + R"(",
      "connections": )" + std::to_string(health.hostedConnections) + R"(,
      "size_mb": )" + std::to_string(health.hostedDbSizeMB) + R"(,
      "active_queries": )" + std::to_string(health.hostedActiveQueries) + R"(
    }
  }
}
)";
        
        res.set_content(json, "application/json");
        if (!health.overall) {
            res.status = 503;
        }
    });

    int port = config_->getHealthPort(); // Get port from config
    spdlog::info("Starting health server on port {}", port);
    server.listen("0.0.0.0", port);
}

} // namespace SyncLayer::Health