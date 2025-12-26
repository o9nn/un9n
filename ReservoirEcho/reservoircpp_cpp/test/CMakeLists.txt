cmake_minimum_required(VERSION 3.14)

# Find Google Test
find_package(GTest REQUIRED)
include(GoogleTest)

# Node test
add_executable(node_test node_test.cpp)
target_link_libraries(node_test PRIVATE reservoircpp GTest::GTest GTest::Main)
gtest_discover_tests(node_test)

# Model test
add_executable(model_test model_test.cpp)
target_link_libraries(model_test PRIVATE reservoircpp GTest::GTest GTest::Main)
gtest_discover_tests(model_test)
