#pragma once

#include <memory>
#include <string>
#include <vector>
#include "ChangeEvent.hpp"

namespace SyncLayer {
namespace Config { class Config; }
namespace DB { class DBConnection; }
}

namespace SyncLayer::Tracker {

// use ChangeEvent from the same namespace

class TableTracker {
public:
    TableTracker(SyncLayer::DB::DBConnection* local, std::shared_ptr<SyncLayer::Config::Config> config);
    void discoverTables();
    std::vector<ChangeEvent> fetchChanges(int batchSize);

private:
    SyncLayer::DB::DBConnection* local_;
    std::shared_ptr<SyncLayer::Config::Config> config_;
    std::vector<std::string> trackedTables_;
};

} // namespace SyncLayer::Tracker


