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

struct HealthStatus {
    bool overall;
    bool localDb;
    bool hostedDb;
    long localConnections;
    long hostedConnections;
    long localDbSizeMB;
    long hostedDbSizeMB;
    long localActiveQueries;
    long hostedActiveQueries;
};

class ReplicationManager {
public:
    ReplicationManager(std::shared_ptr<SyncLayer::Config::Config> config,
                       std::shared_ptr<SyncLayer::Logging::Logger> logger);
    void start();
    HealthStatus healthCheck();

private:
    void initialSync();
    PGresult* executeWithRetry(PGconn* conn, const std::string& query, int maxAttempts = 3);
    std::shared_ptr<SyncLayer::Config::Config> config_;
    std::shared_ptr<SyncLayer::Logging::Logger> logger_;
    std::unique_ptr<SyncLayer::DB::DBConnection> local_;
    std::unique_ptr<SyncLayer::DB::DBConnection> hosted_;
    std::unique_ptr<SyncLayer::Tracker::TableTracker> tracker_;
    std::unique_ptr<SyncLayer::Queue::QueueHandler> queue_;
    bool initialSyncDone_;
};

} // namespace SyncLayer::Replication


