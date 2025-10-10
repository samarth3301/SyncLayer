#include "tracker/TableTracker.hpp"
#include "config/Config.hpp"
#include "db/DBConnection.hpp"
#include <spdlog/spdlog.h>

namespace SyncLayer::Tracker {

TableTracker::TableTracker(SyncLayer::DB::DBConnection* local, std::shared_ptr<SyncLayer::Config::Config> config)
    : local_(local), config_(std::move(config)) {}

void TableTracker::discoverTables()
{
    trackedTables_.clear();
    auto cfgTables = config_->getTables();
    if (!cfgTables.empty()) {
        trackedTables_ = cfgTables;
    } else {
        // Placeholder: in production, query information_schema
        trackedTables_.push_back("public.sample");
    }
    spdlog::info("Tracking {} tables", trackedTables_.size());
}

std::vector<ChangeEvent> TableTracker::fetchChanges(int batchSize)
{
    // Placeholder stub: in production, use logical decoding or triggers
    std::vector<ChangeEvent> events;
    for (int i = 0; i < batchSize && i < 3; ++i) {
        events.push_back( ChangeEvent{ trackedTables_.empty() ? "public.sample" : trackedTables_.front(), "insert", "{\"id\":1}" } );
    }
    return events;
}

} // namespace SyncLayer::Tracker


