cmake_minimum_required(VERSION 3.10)
project(parallel-algorithms)
set(CMAKE_CXX_STANDARD 17)

link_directories(./lib)
include_directories(./include)

include_directories(./src/include)

add_compile_options(-fcilkplus)

add_executable(bench_scan.out src/scan.cpp benchmarks/bench_scan.cpp)
target_link_libraries(bench_scan.out pthread cilkrts)

add_executable(bench_map_parallel.out benchmarks/bench_map_parallel.cpp)
target_link_libraries(bench_map_parallel.out pthread cilkrts)

add_executable(bench_filter_parallel.out benchmarks/bench_filter_parallel.cpp src/scan.cpp)
target_link_libraries(bench_filter_parallel.out pthread cilkrts)

add_executable(bench_sort.out benchmarks/bench_sort.cpp src/scan.cpp)
target_link_libraries(bench_sort.out pthread cilkrts)

add_subdirectory(tests)
