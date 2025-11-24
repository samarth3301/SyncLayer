#include <core/Engine.hpp>
#include <config/Config.hpp>
#include <logging/Logger.hpp>
#include <replication/ReplicationManager.hpp>
#include <health/HealthServer.hpp>
#include <spdlog/spdlog.h>

using SyncLayer::Config::Config;
using SyncLayer::Logging::Logger;
using SyncLayer::Replication::ReplicationManager;
using SyncLayer::Health::HealthServer;

namespace SyncLayer::Core {

Engine::Engine(const std::string& configPath)
{
    config_ = std::make_shared<SyncLayer::Config::Config>(configPath);
    logger_ = std::make_shared<Logger>(config_->getLogLevel(), config_->getLogFile());
    
    try {
        replicationManager_ = std::make_unique<ReplicationManager>(config_, logger_);
    } catch (const std::exception& e) {
        spdlog::warn("Failed to initialize replication manager: {}", e.what());
        spdlog::info("Health server will still start, but database operations will be unavailable");
    }
    
    healthServer_ = std::make_unique<HealthServer>(config_, logger_, replicationManager_);
    healthServer_->start();
}

Engine::~Engine() {
    if (healthServer_) {
        healthServer_->stop();
    }
}

void Engine::run()
{
    spdlog::info("Engine run start");
    if (replicationManager_) {
        replicationManager_->start();
    } else {
        spdlog::warn("Replication manager not available, running in health-check only mode");
    }
}

} // namespace SyncLayer::Core


