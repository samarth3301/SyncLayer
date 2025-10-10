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
    tablePrimaryKeys_.clear();
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

    // Discover primary keys for each table
    for (const auto& table : trackedTables_) {
        size_t dotPos = table.find('.');
        std::string schema = table.substr(0, dotPos);
        std::string tableName = table.substr(dotPos + 1);
        std::string pkQuery = "SELECT kcu.column_name FROM information_schema.table_constraints tc "
                              "JOIN information_schema.key_column_usage kcu ON tc.constraint_name = kcu.constraint_name "
                              "WHERE tc.table_name = '" + tableName + "' AND tc.table_schema = '" + schema + "' AND tc.constraint_type = 'PRIMARY KEY' "
                              "ORDER BY kcu.ordinal_position";
        PGresult* pkRes = PQexec(local_->raw(), pkQuery.c_str());
        if (PQresultStatus(pkRes) == PGRES_TUPLES_OK) {
            int nPkRows = PQntuples(pkRes);
            std::vector<std::string> pkColumns;
            for (int i = 0; i < nPkRows; ++i) {
                pkColumns.push_back(PQgetvalue(pkRes, i, 0));
            }
            tablePrimaryKeys_[table] = pkColumns;
            if (pkColumns.empty()) {
                spdlog::warn("No primary key found for table {}", table);
            }
        } else {
            spdlog::error("Failed to get primary key for {}: {}", table, PQerrorMessage(local_->raw()));
        }
        PQclear(pkRes);
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

const std::vector<std::string>& TableTracker::getPrimaryKeys(const std::string& table) const
{
    static const std::vector<std::string> empty;
    auto it = tablePrimaryKeys_.find(table);
    return it != tablePrimaryKeys_.end() ? it->second : empty;
}

} // namespace SyncLayer::Tracker


