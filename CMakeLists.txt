cmake_minimum_required(VERSION 3.14)
project(un9n VERSION 0.1.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type defaults to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find Eigen - check multiple locations
# Priority 1: System-installed Eigen3
find_package(Eigen3 3.3 QUIET)

if(NOT EIGEN3_FOUND)
    # Priority 2: Bundled Eigen 3.4.0 (check if complete)
    set(EIGEN_3_4_DIR "${CMAKE_SOURCE_DIR}/ReservoirEcho/external/eigen-3.4.0")
    if(EXISTS "${EIGEN_3_4_DIR}/Eigen/Core")
        set(EIGEN3_INCLUDE_DIR "${EIGEN_3_4_DIR}")
        set(EIGEN3_FOUND TRUE)
        message(STATUS "Using bundled Eigen 3.4.0 from ${EIGEN3_INCLUDE_DIR}")
    endif()
endif()

if(NOT EIGEN3_FOUND)
    # Priority 3: Taskflow bundled Eigen 3.3.7 (known to be complete)
    set(EIGEN_3_3_DIR "${CMAKE_SOURCE_DIR}/ReservoirEcho/external/taskflow-3.8.0/3rd-party/eigen-3.3.7")
    if(EXISTS "${EIGEN_3_3_DIR}/Eigen/Core")
        set(EIGEN3_INCLUDE_DIR "${EIGEN_3_3_DIR}")
        set(EIGEN3_FOUND TRUE)
        message(STATUS "Using bundled Eigen 3.3.7 from ${EIGEN3_INCLUDE_DIR}")
    endif()
endif()

if(NOT EIGEN3_FOUND)
    message(FATAL_ERROR "Eigen library not found. Install libeigen3-dev or ensure bundled Eigen has header files.")
endif()

# Create an interface library for Eigen
add_library(Eigen3::Eigen INTERFACE IMPORTED)
set_target_properties(Eigen3::Eigen PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${EIGEN3_INCLUDE_DIR}"
)

# Set compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
    # Suppress deprecated-copy warnings from Eigen 3.3.7 with modern GCC
    add_compile_options(-Wno-deprecated-copy)
endif()

# Add ReservoirCpp subdirectory
add_subdirectory(ReservoirEcho/reservoircpp_cpp)

message(STATUS "un9n project configured successfully")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Eigen location: ${EIGEN3_INCLUDE_DIR}")
