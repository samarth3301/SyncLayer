#pragma once

#include <memory>
#include <thread>
#include <string>
#include <replication/ReplicationManager.hpp>

namespace SyncLayer {
namespace Config { class Config; }
namespace Logging { class Logger; }
}

namespace SyncLayer::Health {

class HealthServer {
public:
    HealthServer(std::shared_ptr<SyncLayer::Config::Config> config,
                 std::shared_ptr<SyncLayer::Logging::Logger> logger,
                 std::unique_ptr<SyncLayer::Replication::ReplicationManager>& replicationManager);
    ~HealthServer();
    void start();
    void stop();

private:
    void runServer();
    std::shared_ptr<SyncLayer::Config::Config> config_;
    std::shared_ptr<SyncLayer::Logging::Logger> logger_;
    std::unique_ptr<SyncLayer::Replication::ReplicationManager>& replicationManager_;
    std::thread serverThread_;
    bool running_;
};

} // namespace SyncLayer::Health