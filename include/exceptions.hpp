#pragma once

#include <stdexcept>
#include <string>

namespace SyncLayer::Exception {

/**
 * @brief Base class for all custom SyncLayer exceptions.
 */
class BaseException : public std::runtime_error {
public:
    // Pass the message to the base std::runtime_error constructor
    BaseException(const std::string& msg) 
        : std::runtime_error("SyncLayer Error: " + msg) {}
};

/**
 * @brief Exception for configuration file errors (e.g., file not found, bad format).
 */
class ConfigurationError : public BaseException {
public:
    ConfigurationError(const std::string& msg)
        : BaseException("Configuration Error: " + msg) {}
};

/**
 * @brief Exception for database-related issues.
 */
class DatabaseError : public BaseException {
public:
    DatabaseError(const std::string& msg)
        : BaseException("Database Error: " + msg) {}
};

/**
 * @brief Exception for replication pipeline failures.
 */
class ReplicationError : public BaseException {
public:
    ReplicationError(const std::string& msg)
        : BaseException("Replication Error: " + msg) {}
};


} // namespace SyncLayer::Exception