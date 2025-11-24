include(GoogleTest)

# Test executables
add_executable(test_config test_config.cpp)
target_link_libraries(test_config gtest_main yaml-cpp spdlog::spdlog)
target_include_directories(test_config PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(test_dbconnection test_dbconnection.cpp)
target_link_libraries(test_dbconnection gtest_main PostgreSQL::PostgreSQL spdlog::spdlog)
target_include_directories(test_dbconnection PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(test_replicationmanager test_replicationmanager.cpp)
target_link_libraries(test_replicationmanager gtest_main PostgreSQL::PostgreSQL yaml-cpp spdlog::spdlog)
target_include_directories(test_replicationmanager PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(test_utils test_utils.cpp)
target_link_libraries(test_utils gtest_main spdlog::spdlog)
target_include_directories(test_utils PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Discover tests
gtest_discover_tests(test_config)
gtest_discover_tests(test_dbconnection)
gtest_discover_tests(test_replicationmanager)
gtest_discover_tests(test_utils)
