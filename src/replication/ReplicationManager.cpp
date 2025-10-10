#include "replication/ReplicationManager.hpp"
#include "config/Config.hpp"
#include "logging/Logger.hpp"
#include "db/DBConnection.hpp"
#include "tracker/TableTracker.hpp"
#include "queue/QueueHandler.hpp"
#include "utils/Retry.hpp"
#include <spdlog/spdlog.h>

namespace SyncLayer::Replication {

ReplicationManager::ReplicationManager(std::shared_ptr<SyncLayer::Config::Config> config,
                                       std::shared_ptr<SyncLayer::Logging::Logger> logger)
    : config_(std::move(config)), logger_(std::move(logger)), initialSyncDone_(false)
{
    local_ = std::make_unique<SyncLayer::DB::DBConnection>(config_->getLocalConnString());
    hosted_ = std::make_unique<SyncLayer::DB::DBConnection>(config_->getHostedConnString());
    tracker_ = std::make_unique<SyncLayer::Tracker::TableTracker>(local_.get(), config_);
    queue_ = std::make_unique<SyncLayer::Queue::QueueHandler>();
}

void ReplicationManager::initialSync()
{
    const auto& tables = tracker_->getTrackedTables();
    spdlog::info("Starting initial data sync for {} tables", tables.size());
    for (const auto& table : tables) {
        // Extract table name, handling schema prefix
        size_t dotPos = table.find_last_of('.');
        std::string tableName = (dotPos != std::string::npos) ? table.substr(dotPos + 1) : table;
        spdlog::info("Syncing data for table: {}", tableName);
        
        std::string query = "SELECT * FROM \"" + tableName + "\"";
        PGresult* res = executeWithRetry(local_->raw(), query);
        if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
            spdlog::error("Failed to select from {} after retries: {}", tableName, res ? PQerrorMessage(local_->raw()) : "No result");
            if (res) PQclear(res);
            continue;
        }
        
        int nFields = PQnfields(res);
        int nRows = PQntuples(res);
        spdlog::info("Table {} has {} rows", tableName, nRows);
        
        if (nRows == 0) continue; // No data to sync
        
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
            insert += ") ON CONFLICT (\"" + std::string(PQfname(res, 0)) + "\") DO UPDATE SET ";
            for (int j = 1; j < nFields; ++j) {
                if (j > 1) insert += ", ";
                insert += "\"" + std::string(PQfname(res, j)) + "\" = EXCLUDED.\"" + std::string(PQfname(res, j)) + "\"";
            }
            
            PGresult* insRes = executeWithRetry(hosted_->raw(), insert);
            if (!insRes || PQresultStatus(insRes) != PGRES_COMMAND_OK) {
                spdlog::error("Failed to upsert into {} after retries: {}", tableName, insRes ? PQerrorMessage(hosted_->raw()) : "No result");
            } else {
                spdlog::info("Upserted row {} into {}", i + 1, tableName);
            }
            if (insRes) PQclear(insRes);
        }
        PQclear(res);
    }
    spdlog::info("Initial data sync completed");
}

void ReplicationManager::start()
{
    spdlog::info("ReplicationManager started. Interval {} sec, batch {}", 
                 config_->getIntervalSeconds(), config_->getBatchSize());
    
    if (!healthCheck()) {
        spdlog::error("Health check failed. Aborting sync.");
        return;
    }
    
    // Discover tables first
    tracker_->discoverTables();
    
    // Perform initial data sync only once
    if (!initialSyncDone_) {
        initialSync();
        initialSyncDone_ = true;
    }
    
    // Then fetch changes
    auto changes = tracker_->fetchChanges(config_->getBatchSize());
    for (const auto& change : changes) {
        queue_->enqueue(change);
    }
    queue_->drainTo(hosted_.get());
}

bool ReplicationManager::healthCheck() {
    spdlog::info("Performing health check on databases...");
    
    PGresult* res = PQexec(local_->raw(), "SELECT 1");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        spdlog::error("Local DB health check failed: {}", PQerrorMessage(local_->raw()));
        PQclear(res);
        return false;
    }
    PQclear(res);
    
    res = PQexec(hosted_->raw(), "SELECT 1");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        spdlog::error("Hosted DB health check failed: {}", PQerrorMessage(hosted_->raw()));
        PQclear(res);
        return false;
    }
    PQclear(res);
    
    spdlog::info("Health check passed.");
    return true;
}

PGresult* ReplicationManager::executeWithRetry(PGconn* conn, const std::string& query, int maxAttempts) {
    PGresult* res = nullptr;
    SyncLayer::Utils::Retry::withExponentialBackoff(maxAttempts, [&](int attempt) {
        res = PQexec(conn, query.c_str());
        if (PQresultStatus(res) == PGRES_COMMAND_OK || PQresultStatus(res) == PGRES_TUPLES_OK) {
            return true; // success
        } else {
            spdlog::warn("Query failed on attempt {}: {}", attempt, PQerrorMessage(conn));
            PQclear(res);
            res = nullptr;
            return false; // retry
        }
    });
    return res;
}

} // namespace SyncLayer::Replication


