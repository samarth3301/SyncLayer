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

void ReplicationManager::initialSync()
{
    spdlog::info("Starting initial data sync for tables");
    const auto& tables = tracker_->getTrackedTables();
    for (const auto& table : tables) {
        // Extract table name, handling schema prefix
        size_t dotPos = table.find_last_of('.');
        std::string tableName = (dotPos != std::string::npos) ? table.substr(dotPos + 1) : table;
        spdlog::info("Syncing data for table: {}", tableName);
        
        std::string query = "SELECT * FROM \"" + tableName + "\"";
        PGresult* res = PQexec(local_->raw(), query.c_str());
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            spdlog::error("Failed to select from {}: {}", tableName, PQerrorMessage(local_->raw()));
            PQclear(res);
            continue;
        }
        
        int nFields = PQnfields(res);
        int nRows = PQntuples(res);
        spdlog::info("Table {} has {} rows", tableName, nRows);
        
        for (int i = 0; i < nRows; ++i) {
            std::string insert = "INSERT INTO \"" + tableName + "\" (";
            for (int j = 0; j < nFields; ++j) {
                if (j > 0) insert += ",";
                insert += "\"" + std::string(PQfname(res, j)) + "\"";
            }
            insert += ") VALUES (";
            for (int j = 0; j < nFields; ++j) {
                if (j > 0) insert += ",";
                if (PQgetisnull(res, i, j)) {
                    insert += "NULL";
                } else {
                    const char* val = PQgetvalue(res, i, j);
                    char* escaped = PQescapeLiteral(hosted_->raw(), val, strlen(val));
                    if (escaped) {
                        insert += escaped;
                        PQfreemem(escaped);
                    } else {
                        insert += "NULL"; // fallback
                    }
                }
            }
            insert += ")";
            
            PGresult* insRes = PQexec(hosted_->raw(), insert.c_str());
            if (PQresultStatus(insRes) != PGRES_COMMAND_OK) {
                spdlog::error("Failed to insert into {}: {}", tableName, PQerrorMessage(hosted_->raw()));
            }
            PQclear(insRes);
        }
        PQclear(res);
    }
    spdlog::info("Initial data sync completed");
}

void ReplicationManager::start()
{
    spdlog::info("ReplicationManager started. Interval {} sec, batch {}", 
                 config_->getIntervalSeconds(), config_->getBatchSize());
    
    // Discover tables first
    tracker_->discoverTables();
    
    // Perform initial data sync
    initialSync();
    
    // Then fetch changes
    auto changes = tracker_->fetchChanges(config_->getBatchSize());
    for (const auto& change : changes) {
        queue_->enqueue(change);
    }
    queue_->drainTo(hosted_.get());
}

} // namespace SyncLayer::Replication


