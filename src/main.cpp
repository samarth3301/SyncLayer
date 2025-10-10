#include "../include/core/Engine.hpp"
#include "../include/exceptions.hpp"
#include <iostream>
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>
#include <cstdlib>

std::string getConfigPath() {
    const char* envPath = std::getenv("SYNC_CONFIG_PATH");
    return envPath && *envPath ? std::string(envPath) : "./config/sync-config.yaml";
}

int main(int argc, char* argv[]) {
    try {
        spdlog::info("SyncLayer microservice starting up...");
        
        // 1. Initialize Configuration
        // 2. Initialize DB Connections
        // 3. Start the Core Engine orchestration loop
        
        SyncLayer::Core::Engine engine(getConfigPath());
        
        while (true) {
            engine.run();
            spdlog::info("Sync completed. Sleeping for 1 hour...");
            std::this_thread::sleep_for(std::chrono::hours(1));
        }
        
    } catch (const SyncLayer::Exception::BaseException& e) {
        spdlog::critical("SyncLayer Critical Error: {}", e.what());
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        spdlog::critical("An unexpected standard exception occurred: {}", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}