#include "replication/ReplicationManager.hpp"
#include "config/Config.hpp"
#include "logging/Logger.hpp"
#include "db/DBConnection.hpp"
#include "tracker/TableTracker.hpp"
#include "queue/QueueHandler.hpp"
#include <spdlog/spdlog.h>

namespace SyncLayer::Replication {

ReplicationManager::ReplicationManager(std::shared_ptr<SyncLayer::Config::Config> config,
                                       std::shared_ptr<SyncLayer::Logging::Logger> logger)
    : config_(std::move(config)), logger_(std::move(logger))
{
    local_ = std::make_unique<SyncLayer::DB::DBConnection>(config_->getLocalConnString());
    hosted_ = std::make_unique<SyncLayer::DB::DBConnection>(config_->getHostedConnString());
    tracker_ = std::make_unique<SyncLayer::Tracker::TableTracker>(local_.get(), config_);
    queue_ = std::make_unique<SyncLayer::Queue::QueueHandler>();
}

void ReplicationManager::start()
{
    spdlog::info("ReplicationManager started. Interval {} sec, batch {}", 
                 config_->getIntervalSeconds(), config_->getBatchSize());
    // Placeholder: discovery and a single tick
    tracker_->discoverTables();
    auto changes = tracker_->fetchChanges(config_->getBatchSize());
    for (const auto& change : changes) {
        queue_->enqueue(change);
    }
    queue_->drainTo(hosted_.get());
}

} // namespace SyncLayer::Replication


