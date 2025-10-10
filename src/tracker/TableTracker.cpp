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
        // Discover all user tables in public schema
        PGresult* res = PQexec(local_->raw(), "SELECT table_name FROM information_schema.tables WHERE table_schema = 'public' AND table_type = 'BASE TABLE'");
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int nRows = PQntuples(res);
            for (int i = 0; i < nRows; ++i) {
                std::string tableName = "public." + std::string(PQgetvalue(res, i, 0));
                trackedTables_.push_back(tableName);
            }
        } else {
            spdlog::error("Failed to discover tables: {}", PQerrorMessage(local_->raw()));
        }
        PQclear(res);
    }
    std::string joinedTables;
    if (!trackedTables_.empty()) {
        joinedTables = trackedTables_[0];
        for (size_t i = 1; i < trackedTables_.size(); ++i) {
            joinedTables += ", " + trackedTables_[i];
        }
    }
    spdlog::info("Tracking {} tables: {}", trackedTables_.size(), joinedTables);
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

const std::vector<std::string>& TableTracker::getTrackedTables() const
{
    return trackedTables_;
}

} // namespace SyncLayer::Tracker


