cmake_minimum_required(VERSION 3.14)
project(ReservoirCpp VERSION 0.1.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Eigen if not already found by parent project
if(NOT EIGEN3_FOUND)
    # Priority 1: Check eigen-3.4.0 (if complete with headers)
    set(EIGEN_3_4_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/eigen-3.4.0")
    if(EXISTS "${EIGEN_3_4_DIR}/Eigen/Core")
        set(EIGEN3_INCLUDE_DIR "${EIGEN_3_4_DIR}")
        set(EIGEN3_FOUND TRUE)
        message(STATUS "ReservoirCpp: Using Eigen 3.4.0 from ${EIGEN3_INCLUDE_DIR}")
    endif()
endif()

if(NOT EIGEN3_FOUND)
    # Priority 2: Check taskflow bundled eigen-3.3.7
    set(EIGEN_3_3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/taskflow-3.8.0/3rd-party/eigen-3.3.7")
    if(EXISTS "${EIGEN_3_3_DIR}/Eigen/Core")
        set(EIGEN3_INCLUDE_DIR "${EIGEN_3_3_DIR}")
        set(EIGEN3_FOUND TRUE)
        message(STATUS "ReservoirCpp: Using Eigen 3.3.7 from ${EIGEN3_INCLUDE_DIR}")
    endif()
endif()

if(NOT EIGEN3_FOUND)
    message(FATAL_ERROR "Eigen library not found. Install libeigen3-dev or ensure bundled Eigen has header files.")
# Find required packages
# First try to find system Eigen using find_package
find_package(Eigen3 3.3 QUIET CONFIG)
if(NOT Eigen3_FOUND)
    # Try again without CONFIG (for older CMake or different installations)
    find_package(Eigen3 3.3 QUIET)
endif()

if(Eigen3_FOUND)
    message(STATUS "Using system Eigen3: ${EIGEN3_INCLUDE_DIR}")
    if(NOT EIGEN3_INCLUDE_DIR)
        # Some Eigen3 configs set EIGEN3_INCLUDE_DIRS instead
        set(EIGEN3_INCLUDE_DIR "${EIGEN3_INCLUDE_DIRS}")
    endif()
    if(NOT EIGEN3_INCLUDE_DIR)
        # Get include dir from target if available
        get_target_property(EIGEN3_INCLUDE_DIR Eigen3::Eigen INTERFACE_INCLUDE_DIRECTORIES)
    endif()
else()
    # Check common system paths for Eigen
    set(EIGEN_SEARCH_PATHS
        "/usr/include/eigen3"
        "/usr/local/include/eigen3"
        "/opt/homebrew/include/eigen3"
        "${CMAKE_CURRENT_SOURCE_DIR}/../external/eigen-3.4.0"
    )
    
    foreach(SEARCH_PATH ${EIGEN_SEARCH_PATHS})
        if(EXISTS "${SEARCH_PATH}/Eigen/Dense")
            set(EIGEN3_INCLUDE_DIR "${SEARCH_PATH}")
            set(Eigen3_FOUND TRUE)
            message(STATUS "Found Eigen at ${EIGEN3_INCLUDE_DIR}")
            break()
        endif()
    endforeach()
    
    if(NOT Eigen3_FOUND)
        message(FATAL_ERROR "Eigen library not found. Install libeigen3-dev or ensure bundled Eigen has header files.")
    endif()
endif()

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
