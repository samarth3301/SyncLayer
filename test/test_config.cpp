#include <gtest/gtest.h>
#include "config/Config.hpp"
#include <filesystem>
#include <fstream>

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary config file
        tempConfigPath = "/tmp/test_config.yaml";
        std::ofstream configFile(tempConfigPath);
        configFile << "databases:\n"
                   << "  local:\n"
                   << "    host: localhost\n"
                   << "    port: 5432\n"
                   << "    user: testuser\n"
                   << "    password: testpass\n"
                   << "    dbname: testdb\n"
                   << "  hosted:\n"
                   << "    host: remotehost\n"
                   << "    port: 5432\n"
                   << "    user: remoteuser\n"
                   << "    password: remotepass\n"
                   << "    dbname: remotedb\n"
                   << "sync:\n"
                   << "  interval_seconds: 10\n"
                   << "  batch_size: 100\n"
                   << "  auto_fetch: true\n"
                   << "  tables: []\n"
                   << "logging:\n"
                   << "  level: info\n"
                   << "  file: test.log\n";
        configFile.close();
    }

    void TearDown() override {
        std::filesystem::remove(tempConfigPath);
    }

    std::string tempConfigPath;
};

TEST_F(ConfigTest, LoadConfigSuccessfully) {
    Config config;
    ASSERT_TRUE(config.loadFromFile(tempConfigPath));

    auto localDb = config.getLocalDatabase();
    EXPECT_EQ(localDb.host, "localhost");
    EXPECT_EQ(localDb.port, 5432);
    EXPECT_EQ(localDb.user, "testuser");
    EXPECT_EQ(localDb.password, "testpass");
    EXPECT_EQ(localDb.dbname, "testdb");

    auto hostedDb = config.getHostedDatabase();
    EXPECT_EQ(hostedDb.host, "remotehost");
    EXPECT_EQ(hostedDb.port, 5432);
    EXPECT_EQ(hostedDb.user, "remoteuser");
    EXPECT_EQ(hostedDb.password, "remotepass");
    EXPECT_EQ(hostedDb.dbname, "remotedb");

    auto syncConfig = config.getSyncConfig();
    EXPECT_EQ(syncConfig.interval_seconds, 10);
    EXPECT_EQ(syncConfig.batch_size, 100);
    EXPECT_TRUE(syncConfig.auto_fetch);

    auto logConfig = config.getLoggingConfig();
    EXPECT_EQ(logConfig.level, "info");
    EXPECT_EQ(logConfig.file, "test.log");
}

TEST_F(ConfigTest, LoadInvalidFile) {
    Config config;
    ASSERT_FALSE(config.loadFromFile("/nonexistent/file.yaml"));
}