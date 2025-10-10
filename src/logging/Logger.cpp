#include "logging/Logger.hpp"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace SyncLayer::Logging {

static spdlog::level::level_enum toLevel(const std::string& lvl) {
    if (lvl == "trace") return spdlog::level::trace;
    if (lvl == "debug") return spdlog::level::debug;
    if (lvl == "info") return spdlog::level::info;
    if (lvl == "warn") return spdlog::level::warn;
    if (lvl == "error") return spdlog::level::err;
    if (lvl == "critical") return spdlog::level::critical;
    return spdlog::level::info;
}

Logger::Logger(const std::string& level, const std::string& filePath) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath, true);
    std::vector<spdlog::sink_ptr> sinks { console_sink, file_sink };
    auto logger = std::make_shared<spdlog::logger>("synclayer", sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger);
    spdlog::set_level(toLevel(level));
    spdlog::flush_on(spdlog::level::info);
}

void Logger::setLevel(const std::string& level) {
    spdlog::set_level(toLevel(level));
}

} // namespace SyncLayer::Logging


