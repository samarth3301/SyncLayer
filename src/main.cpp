#include "../include/core/Engine.hpp"
#include "../include/exceptions.hpp"
#include <iostream>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[]) {
    try {
        spdlog::info("SyncLayer microservice starting up...");
        
        // 1. Initialize Configuration
        // 2. Initialize DB Connections
        // 3. Start the Core Engine orchestration loop
        
        SyncLayer::Core::Engine engine("./config/sync-config.yaml");
        engine.run();
        
    } catch (const SyncLayer::Exception::BaseException& e) {
        spdlog::critical("SyncLayer Critical Error: {}", e.what());
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        spdlog::critical("An unexpected standard exception occurred: {}", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}