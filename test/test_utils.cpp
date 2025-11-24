#include <gtest/gtest.h>
#include "utils/Retry.hpp"
#include <chrono>
#include <thread>

TEST(RetryTest, SuccessOnFirstTry) {
    int attempts = 0;
    auto result = retryWithBackoff(
        [&]() {
            attempts++;
            return 42;
        },
        3, std::chrono::milliseconds(10)
    );
    EXPECT_EQ(result, 42);
    EXPECT_EQ(attempts, 1);
}

TEST(RetryTest, SuccessAfterRetries) {
    int attempts = 0;
    auto result = retryWithBackoff(
        [&]() -> int {
            attempts++;
            if (attempts < 3) {
                throw std::runtime_error("Temporary failure");
            }
            return 42;
        },
        3, std::chrono::milliseconds(10)
    );
    EXPECT_EQ(result, 42);
    EXPECT_EQ(attempts, 3);
}

TEST(RetryTest, FailAfterMaxRetries) {
    int attempts = 0;
    EXPECT_THROW(
        retryWithBackoff(
            [&]() -> int {
                attempts++;
                throw std::runtime_error("Persistent failure");
            },
            3, std::chrono::milliseconds(10)
        ),
        std::runtime_error
    );
    EXPECT_EQ(attempts, 3);
}