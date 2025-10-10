#pragma once

#include <memory>
#include <string>

namespace SyncLayer::Logging {

class Logger {
public:
    Logger(const std::string& level, const std::string& filePath);
    void setLevel(const std::string& level);
};

} // namespace SyncLayer::Logging


