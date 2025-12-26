cmake_minimum_required(VERSION 3.14)
project(ReservoirCpp VERSION 0.1.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
# Look for Eigen in our external directory
set(EIGEN3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/eigen-3.4.0")
if(NOT EXISTS "${EIGEN3_INCLUDE_DIR}")
    message(FATAL_ERROR "Eigen library not found at ${EIGEN3_INCLUDE_DIR}")
endif()
set(EIGEN3_FOUND TRUE)

# Set compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${EIGEN3_INCLUDE_DIR}
)

# Core source files (only the ones that compile cleanly)
set(CORE_SOURCES
    src/_base.cpp
    src/_version.cpp
    src/activationsfunc.cpp
)

# Deep Tree Echo source files
set(DEEP_TREE_ECHO_SOURCES
    src/deep_tree_echo/core.cpp
    src/deep_tree_echo/unreal_integration.cpp
    src/deep_tree_echo/avatar_4e.cpp
)

# Add library with core sources only
add_library(reservoircpp_core ${CORE_SOURCES})
target_include_directories(reservoircpp_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Deep Tree Echo integration library
add_library(deep_tree_echo ${DEEP_TREE_ECHO_SOURCES})
target_include_directories(deep_tree_echo PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(deep_tree_echo PUBLIC reservoircpp_core)

# Add tests
option(BUILD_TESTS "Build the tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    # add_subdirectory(test)
endif()

# Add examples
option(BUILD_EXAMPLES "Build the examples" OFF)
if(BUILD_EXAMPLES)
    # add_subdirectory(examples)
endif()

# Installation
install(TARGETS reservoircpp_core deep_tree_echo
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(DIRECTORY include/ DESTINATION include)

message(STATUS "ReservoirCpp Core configured successfully")
message(STATUS "Deep Tree Echo integration enabled")
