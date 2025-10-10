#include "config/Config.hpp"
#include "exceptions.hpp"
#include <yaml-cpp/yaml.h>
#include <sstream>
#include <cstdlib>
#include <vector>

namespace SyncLayer::Config {

static std::string envOr(const char* name, const std::string& fallback) {
    const char* v = std::getenv(name);
    return v && *v ? std::string(v) : fallback;
}

static int envOrInt(const char* name, int fallback) {
    const char* v = std::getenv(name);
    if (v && *v) {
        try { return std::stoi(v); } catch (...) { return fallback; }
    }
    return fallback;
}

static std::vector<std::string> envOrList(const char* name, const std::vector<std::string>& fallback) {
    const char* v = std::getenv(name);
    if (!(v && *v)) return fallback;
    std::vector<std::string> out;
    std::string s(v);
    std::string token;
    std::istringstream ss(s);
    while (std::getline(ss, token, ',')) {
        if (!token.empty()) out.push_back(token);
    }
    return out;
}

static std::string makeConn(const YAML::Node& node,
                            const char* envPrefix) {
    const std::string host = envOr((std::string(envPrefix) + "_HOST").c_str(), node["host"].as<std::string>("localhost"));
    const int port = envOrInt((std::string(envPrefix) + "_PORT").c_str(), node["port"].as<int>(5432));
    const std::string user = envOr((std::string(envPrefix) + "_USER").c_str(), node["user"].as<std::string>("postgres"));
    const std::string password = envOr((std::string(envPrefix) + "_PASSWORD").c_str(), node["password"].as<std::string>(""));
    const std::string dbname = envOr((std::string(envPrefix) + "_DBNAME").c_str(), node["dbname"].as<std::string>("postgres"));
    const std::string sslmode = envOr((std::string(envPrefix) + "_SSLMODE").c_str(), node["sslmode"].as<std::string>("disable"));
    std::ostringstream ss;
    ss << "host=" << host
       << " port=" << port
       << " user=" << user
       << " password=" << password
       << " dbname=" << dbname
       << " sslmode=" << sslmode;
    return ss.str();
}

Config::Config(const std::string& path) {
    loadFromFile(path);
}

void Config::loadFromFile(const std::string& path) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        const auto dbs = root["databases"];
        localConn_ = makeConn(dbs["local"], "SYNC_LOCAL");
        hostedConn_ = makeConn(dbs["hosted"], "SYNC_HOSTED");

        const auto sync = root["sync"];
        intervalSeconds_ = envOrInt("SYNC_INTERVAL_SECONDS", sync["interval_seconds"].as<int>(5));
        batchSize_ = envOrInt("SYNC_BATCH_SIZE", sync["batch_size"].as<int>(50));
        std::vector<std::string> yamlTables;
        if (sync["tables"]) {
            for (const auto& t : sync["tables"]) {
                yamlTables.push_back(t.as<std::string>());
            }
        }
        tables_ = envOrList("SYNC_TABLES", yamlTables);

        const auto log = root["logging"];
        logLevel_ = envOr("SYNC_LOG_LEVEL", log["level"].as<std::string>("info"));
        logFile_ = envOr("SYNC_LOG_FILE", log["file"].as<std::string>("logs/synclayer.log"));
    } catch (const YAML::BadFile&) {
        throw SyncLayer::Exception::ConfigurationError("Config file not found: " + path);
    } catch (const std::exception& e) {
        throw SyncLayer::Exception::ConfigurationError(e.what());
    }
}

std::string Config::getLocalConnString() const { return localConn_; }
std::string Config::getHostedConnString() const { return hostedConn_; }
int Config::getIntervalSeconds() const { return intervalSeconds_; }
int Config::getBatchSize() const { return batchSize_; }
std::vector<std::string> Config::getTables() const { return tables_; }
std::string Config::getLogLevel() const { return logLevel_; }
std::string Config::getLogFile() const { return logFile_; }

} // namespace SyncLayer::Config


