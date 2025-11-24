#include "replication/ReplicationManager.hpp"
#include "config/Config.hpp"
#include "logging/Logger.hpp"
#include "db/DBConnection.hpp"
#include "tracker/TableTracker.hpp"
#include "queue/QueueHandler.hpp"
#include "utils/Retry.hpp"
#include <spdlog/spdlog.h>
#include <cstring>

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
    const int pageSize = 1000;
    const int batchSize = 100; // Batch inserts
    for (const auto& table : tables) {
        const auto& pk = tracker_->getPrimaryKeys(table);
        if (pk.empty()) {
            spdlog::warn("Skipping table {} due to no primary key", table);
            continue;
        }
        spdlog::info("Syncing data for table: {}", table);
        
        std::string orderBy = " ORDER BY ";
        for (size_t i = 0; i < pk.size(); ++i) {
            if (i > 0) orderBy += ", ";
            orderBy += "\"" + pk[i] + "\"";
        }
        
        int offset = 0;
        int totalRows = 0;
        while (true) {
            std::string query = "SELECT * FROM " + table + orderBy + " LIMIT " + std::to_string(pageSize) + " OFFSET " + std::to_string(offset);
            PGresult* res = executeWithRetry(local_->raw(), query);
            if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
                spdlog::error("Failed to select from {}: {}", table, res ? PQerrorMessage(local_->raw()) : "No result");
                if (res) PQclear(res);
                break;
            }
            
            int nRows = PQntuples(res);
            if (nRows == 0) {
                PQclear(res);
                break;
            }
            
            int nFields = PQnfields(res);
            std::vector<std::string> inserts;
            for (int i = 0; i < nRows; ++i) {
                std::string values = "(";
                for (int j = 0; j < nFields; ++j) {
                    if (j > 0) values += ",";
                    if (PQgetisnull(res, i, j)) {
                        values += "NULL";
                    } else {
                        const char* val = PQgetvalue(res, i, j);
                        char* escaped = PQescapeLiteral(hosted_->raw(), val, strlen(val));
                        if (escaped) {
                            values += escaped;
                            PQfreemem(escaped);
                        } else {
                            values += "NULL";
                        }
                    }
                }
                values += ")";
                inserts.push_back(values);
                
                if (inserts.size() >= batchSize) {
                    std::string insertQuery = "INSERT INTO " + table + " (";
                    for (int j = 0; j < nFields; ++j) {
                        if (j > 0) insertQuery += ",";
                        insertQuery += "\"" + std::string(PQfname(res, j)) + "\"";
                    }
                    insertQuery += ") VALUES ";
                    for (size_t k = 0; k < inserts.size(); ++k) {
                        if (k > 0) insertQuery += ",";
                        insertQuery += inserts[k];
                    }
                    insertQuery += " ON CONFLICT DO NOTHING";
                    
                    PGresult* insRes = executeWithRetry(hosted_->raw(), insertQuery);
                    if (!insRes || PQresultStatus(insRes) != PGRES_COMMAND_OK) {
                        spdlog::error("Failed to batch insert into {}: {}", table, insRes ? PQerrorMessage(hosted_->raw()) : "No result");
                    }
                    if (insRes) PQclear(insRes);
                    inserts.clear();
                }
            }
            // Insert remaining
            if (!inserts.empty()) {
                std::string insertQuery = "INSERT INTO " + table + " (";
                for (int j = 0; j < nFields; ++j) {
                    if (j > 0) insertQuery += ",";
                    insertQuery += "\"" + std::string(PQfname(res, j)) + "\"";
                }
                insertQuery += ") VALUES ";
                for (size_t k = 0; k < inserts.size(); ++k) {
                    if (k > 0) insertQuery += ",";
                    insertQuery += inserts[k];
                }
                insertQuery += " ON CONFLICT DO NOTHING";
                
                PGresult* insRes = executeWithRetry(hosted_->raw(), insertQuery);
                if (!insRes || PQresultStatus(insRes) != PGRES_COMMAND_OK) {
                    spdlog::error("Failed to batch insert into {}: {}", table, insRes ? PQerrorMessage(hosted_->raw()) : "No result");
                }
                if (insRes) PQclear(insRes);
            }
            
            PQclear(res);
            totalRows += nRows;
            offset += pageSize;
            spdlog::info("Synced {} rows for table {} (offset {})", nRows, table, offset);
        }
        spdlog::info("Completed syncing {} rows for table {}", totalRows, table);
    }
    spdlog::info("Initial data sync completed");
}

void ReplicationManager::start()
{
    spdlog::info("ReplicationManager started. Interval {} sec, batch {}", 
                 config_->getIntervalSeconds(), config_->getBatchSize());
    
    auto health = healthCheck();
    if (!health.overall) {
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

HealthStatus ReplicationManager::healthCheck() {
    spdlog::info("Performing health check on databases...");
    
    HealthStatus status;
    status.localDb = false;
    status.hostedDb = false;
    status.localConnections = 0;
    status.hostedConnections = 0;
    status.localDbSizeMB = 0;
    status.hostedDbSizeMB = 0;
    status.localActiveQueries = 0;
    status.hostedActiveQueries = 0;
    
    // Check local DB
    PGresult* res = PQexec(local_->raw(), "SELECT 1");
    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        status.localDb = true;
        
        // Get connections
        PQclear(res);
        res = PQexec(local_->raw(), "SELECT count(*) FROM pg_stat_activity WHERE datname = current_database()");
        if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
            status.localConnections = atol(PQgetvalue(res, 0, 0));
        }
        
        // Get DB size
        PQclear(res);
        res = PQexec(local_->raw(), "SELECT pg_database_size(current_database()) / 1024 / 1024");
        if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
            status.localDbSizeMB = atol(PQgetvalue(res, 0, 0));
        }
        
        // Get active queries
        PQclear(res);
        res = PQexec(local_->raw(), "SELECT count(*) FROM pg_stat_activity WHERE state = 'active' AND datname = current_database()");
        if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
            status.localActiveQueries = atol(PQgetvalue(res, 0, 0));
        }
    } else {
        spdlog::error("Local DB health check failed: {}", PQerrorMessage(local_->raw()));
    }
    PQclear(res);
    
    // Check hosted DB
    res = PQexec(hosted_->raw(), "SELECT 1");
    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        status.hostedDb = true;
        
        // Get connections
        PQclear(res);
        res = PQexec(hosted_->raw(), "SELECT count(*) FROM pg_stat_activity WHERE datname = current_database()");
        if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
            status.hostedConnections = atol(PQgetvalue(res, 0, 0));
        }
        
        // Get DB size
        PQclear(res);
        res = PQexec(hosted_->raw(), "SELECT pg_database_size(current_database()) / 1024 / 1024");
        if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
            status.hostedDbSizeMB = atol(PQgetvalue(res, 0, 0));
        }
        
        // Get active queries
        PQclear(res);
        res = PQexec(hosted_->raw(), "SELECT count(*) FROM pg_stat_activity WHERE state = 'active' AND datname = current_database()");
        if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
            status.hostedActiveQueries = atol(PQgetvalue(res, 0, 0));
        }
    } else {
        spdlog::error("Hosted DB health check failed: {}", PQerrorMessage(hosted_->raw()));
    }
    PQclear(res);
    
    status.overall = status.localDb && status.hostedDb;
    if (status.overall) {
        spdlog::info("Health check passed.");
    } else {
        spdlog::warn("Health check failed.");
    }
    return status;
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


