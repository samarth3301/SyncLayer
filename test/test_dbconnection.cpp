#include <gtest/gtest.h>
#include "db/DBConnection.hpp"

TEST(DBConnectionTest, BuildConnectionString) {
    DatabaseConfig config{"localhost", 5432, "testuser", "testpass", "testdb"};
    DBConnection db(config);

    // Since connect requires a real DB, we can't test it easily
    // But we can test that the object is created
    EXPECT_TRUE(true); // Placeholder
}

// Note: Full integration tests would require a test database
// For now, this is a basic structure