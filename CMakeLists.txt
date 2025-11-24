cmake_minimum_required(VERSION 3.16)
project(SyncLayer LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")

include_directories(
    include
)

find_package(PkgConfig REQUIRED)
find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(PostgreSQL REQUIRED)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(SRCS
    src/main.cpp
    src/core/Engine.cpp
    src/config/Config.cpp
    src/logging/Logger.cpp
    src/db/DBConnection.cpp
    src/replication/ReplicationManager.cpp
    src/tracker/TableTracker.cpp
    src/queue/QueueHandler.cpp
    src/utils/Retry.cpp
    src/health/HealthServer.cpp
)

add_executable(SyncLayer ${SRCS})


target_include_directories(SyncLayer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PostgreSQL_INCLUDE_DIRS}
)

target_link_libraries(SyncLayer PUBLIC
    yaml-cpp
    spdlog::spdlog
    PostgreSQL::PostgreSQL
)

enable_testing()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test)
add_subdirectory(test)
endif()