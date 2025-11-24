#pragma once

#include <string>
#include <vector>

namespace SyncLayer::Config {

class Config {
public:
    explicit Config(const std::string& path);

    // Accessors
    std::string getLocalConnString() const;
    std::string getHostedConnString() const;

    int getIntervalSeconds() const;
    int getBatchSize() const;
    bool getAutoFetch() const;
    std::vector<std::string> getTables() const;

    std::string getLogLevel() const;
    std::string getLogFile() const;

    int getHealthPort() const;

private:
    void loadFromFile(const std::string& path);

    // cached values
    std::string localConn_;
    std::string hostedConn_;
    int intervalSeconds_ {5};
    int batchSize_ {50};
    bool autoFetch_ {true};
    std::vector<std::string> tables_;
    std::string logLevel_ {"info"};
    std::string logFile_ {"logs/synclayer.log"};
    int healthPort_ {8080};
};

} // namespace SyncLayer::Config


