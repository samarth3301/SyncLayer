#include <core/Engine.hpp>
#include <config/Config.hpp>
#include <logging/Logger.hpp>
#include <replication/ReplicationManager.hpp>
#include <spdlog/spdlog.h>

using SyncLayer::Config::Config;
using SyncLayer::Logging::Logger;
using SyncLayer::Replication::ReplicationManager;

namespace SyncLayer::Core {

Engine::Engine(const std::string& configPath)
{
    config_ = std::make_shared<SyncLayer::Config::Config>(configPath);
    logger_ = std::make_shared<Logger>(config_->getLogLevel(), config_->getLogFile());
    replicationManager_ = std::make_unique<ReplicationManager>(config_, logger_);
}

void Engine::run()
{
    spdlog::info("Engine run start");
    replicationManager_->start();
}

} // namespace SyncLayer::Core


