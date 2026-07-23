# Benchmarks CMakeLists.txt

add_executable(benchmarks
    bench_main.cpp
)

target_link_libraries(benchmarks opencog_core)
