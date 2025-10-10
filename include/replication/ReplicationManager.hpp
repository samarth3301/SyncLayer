#pragma once

#include <memory>
#include <db/DBConnection.hpp>
#include <tracker/TableTracker.hpp>
#include <queue/QueueHandler.hpp>

namespace SyncLayer {
namespace Config { class Config; }
namespace Logging { class Logger; }
}

namespace SyncLayer::Replication {

class ReplicationManager {
public:
    ReplicationManager(std::shared_ptr<SyncLayer::Config::Config> config,
                       std::shared_ptr<SyncLayer::Logging::Logger> logger);
    void start();

private:
    std::shared_ptr<SyncLayer::Config::Config> config_;
    std::shared_ptr<SyncLayer::Logging::Logger> logger_;
    std::unique_ptr<SyncLayer::DB::DBConnection> local_;
    std::unique_ptr<SyncLayer::DB::DBConnection> hosted_;
    std::unique_ptr<SyncLayer::Tracker::TableTracker> tracker_;
    std::unique_ptr<SyncLayer::Queue::QueueHandler> queue_;
};

} // namespace SyncLayer::Replication


