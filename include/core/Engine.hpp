#pragma once

#include <memory>
#include <string>
#include <replication/ReplicationManager.hpp>
#include <health/HealthServer.hpp>

namespace SyncLayer {
namespace Config { class Config; }
namespace Logging { class Logger; }
}

namespace SyncLayer::Core {

class Engine {
public:
    explicit Engine(const std::string& configPath);
    ~Engine();
    void run();

private:
    std::shared_ptr<SyncLayer::Config::Config> config_;
    std::shared_ptr<SyncLayer::Logging::Logger> logger_;
    std::unique_ptr<SyncLayer::Replication::ReplicationManager> replicationManager_;
    std::unique_ptr<SyncLayer::Health::HealthServer> healthServer_;
};

} // namespace SyncLayer::Core


