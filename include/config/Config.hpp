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
    std::vector<std::string> getTables() const;

    std::string getLogLevel() const;
    std::string getLogFile() const;

private:
    void loadFromFile(const std::string& path);

    // cached values
    std::string localConn_;
    std::string hostedConn_;
    int intervalSeconds_ {5};
    int batchSize_ {50};
    std::vector<std::string> tables_;
    std::string logLevel_ {"info"};
    std::string logFile_ {"logs/synclayer.log"};
};

} // namespace SyncLayer::Config


