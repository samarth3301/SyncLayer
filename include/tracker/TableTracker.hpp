#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
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
    const std::vector<std::string>& getTrackedTables() const;
    const std::vector<std::string>& getPrimaryKeys(const std::string& table) const;

private:
    SyncLayer::DB::DBConnection* local_;
    std::shared_ptr<SyncLayer::Config::Config> config_;
    std::vector<std::string> trackedTables_;
    std::map<std::string, std::vector<std::string>> tablePrimaryKeys_;
};

} // namespace SyncLayer::Tracker


