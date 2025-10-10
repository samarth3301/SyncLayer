cmake_minimum_required(VERSION 3.16)
project(SyncLayer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(
    include
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)
pkg_check_modules(LIBPQ REQUIRED libpq)
find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)

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
)

add_executable(SyncLayer ${SRCS})


target_include_directories(SyncLayer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LIBPQ_INCLUDE_DIRS}
    ${YAMLCPP_INCLUDE_DIRS}
)

target_link_libraries(SyncLayer PUBLIC
    ${LIBPQ_LIBRARIES}
    yaml-cpp
    spdlog::spdlog
)

enable_testing()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test)
add_subdirectory(test)
endif()