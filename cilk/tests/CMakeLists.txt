project(tests)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(sort_tests.out 
    test_scan.cpp ../src/scan.cpp
    test_raw_array.cpp
    test_map_parallel.cpp
    test_filter_parallel.cpp
    test_sort_parallel.cpp
)
target_link_libraries(sort_tests.out pthread cilkrts gtest gtest_main)

add_executable(bfs_tests.out bfs_tests.cpp)
target_link_libraries(bfs_tests.out pthread cilkrts gtest gtest_main)