cmake_minimum_required(VERSION 3.10)
project(parallel-algorithms)
set(CMAKE_CXX_STANDARD 17)

link_directories(./lib)
include_directories(./include)
include_directories(./src/include)
include_directories(./)

add_compile_options(-fcilkplus)

add_executable(bench_prefix_sum.out src/prefix_sum.cpp benchmarks/bench_prefix_sum.cpp)
target_link_libraries(bench_prefix_sum.out pthread cilkrts)

add_subdirectory(tests)
