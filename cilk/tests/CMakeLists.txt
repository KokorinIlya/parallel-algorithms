project(tests)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(run_tests.out 
    test_prefix_sum.cpp ../src/prefix_sum.cpp
    test_raw_array.cpp
    test_map_parallel.cpp
)
target_link_libraries(run_tests.out pthread cilkrts gtest gtest_main)