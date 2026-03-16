cmake_minimum_required(VERSION 3.20)
project(opencog_modern
    VERSION 0.1.0
    DESCRIPTION "Modern C++23 OpenCog Implementation"
    LANGUAGES CXX C
)

# Require C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -march=native                    # Enable all CPU features (SIMD)
        -ffast-math                      # Aggressive floating point
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-flto>       # Link-time optimization
        $<$<CONFIG:Debug>:-g3>
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    )
    add_link_options(
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    )
elseif(MSVC)
    add_compile_options(
        /W4
        /arch:AVX2                       # SIMD on Windows
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/GL>         # Whole program optimization
    )
endif()

# Options
option(OPENCOG_BUILD_TESTS "Build test suite" ON)
option(OPENCOG_BUILD_BENCHMARKS "Build benchmarks" ON)
option(OPENCOG_USE_MIMALLOC "Use mimalloc allocator" OFF)
option(OPENCOG_ENABLE_SIMD "Enable SIMD optimizations" ON)

# Find dependencies
find_package(Threads REQUIRED)

# Optional: mimalloc for better allocation performance
if(OPENCOG_USE_MIMALLOC)
    find_package(mimalloc CONFIG)
    if(mimalloc_FOUND)
        add_compile_definitions(OPENCOG_USE_MIMALLOC)
    endif()
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library
add_library(opencog_core STATIC
    src/core/types.cpp
    src/core/memory.cpp
    src/atomspace/atomspace.cpp
    src/atomspace/atom_table.cpp
    src/atomspace/index.cpp
    src/attention/attention_bank.cpp
    src/attention/ecan.cpp
    src/pattern/pattern.cpp
    src/pattern/matcher.cpp
    src/pln/truth_value.cpp
    src/pln/inference.cpp
    src/pln/formulas.cpp
    src/ure/rule.cpp
    src/ure/engine.cpp
    src/endocrine/types.cpp
    src/endocrine/hormone_bus.cpp
    src/endocrine/gland.cpp
    src/endocrine/connector.cpp
    src/endocrine/valence.cpp
    src/endocrine/affect.cpp
    src/endocrine/moral.cpp
    src/endocrine/endocrine_system.cpp
    src/endocrine/npu_adapter.cpp
    src/endocrine/o9c2_adapter.cpp
    src/endocrine/marduk_adapter.cpp
    src/endocrine/touchpad_adapter.cpp
    src/endocrine/guidance_connector.cpp
    src/atomese/loader.cpp
    src/nervous/nerve_bus.cpp
    src/nervous/nucleus.cpp
    src/nervous/nervous_system.cpp
    src/temporal/crystal_bus.cpp
    src/temporal/temporal_system.cpp
    src/entelechy/cloninger.cpp
    src/entelechy/interoceptive.cpp
    src/entelechy/developmental.cpp
    src/entelechy/narrative.cpp
    src/entelechy/social.cpp
    src/entelechy/district.cpp
    src/entelechy/civic_angel.cpp
    src/entelechy/adapter.cpp
    src/afi/blanket.cpp
    src/afi/free_energy.cpp
    src/afi/precision.cpp
    src/afi/adapter.cpp
)

target_link_libraries(opencog_core
    PUBLIC Threads::Threads
    $<$<BOOL:${mimalloc_FOUND}>:mimalloc>
)

# Set properties for coroutines
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(opencog_core PUBLIC -fcoroutines)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(opencog_core PUBLIC -fcoroutines)
endif()

# Tests
if(OPENCOG_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Benchmarks
if(OPENCOG_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# Installation
install(TARGETS opencog_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/opencog DESTINATION include)
