#include <gtest/gtest.h>
#include "sort_parallel.h"
#include <cstdint>
#include <random>
#include <vector>
#include <algorithm>
#include <iostream>

TEST(parallel_sort, simple)
{
    std::vector<int32_t> v({1, 3, 3, 7, -2, 5, 2, 4, 6, -8});
    raw_array<int32_t> arr(v.size());
    for (uint32_t i = 0; i < v.size(); ++i)
    {
        arr[i] = v[i];
    }
    sort_parallel<int32_t>(arr, 3);
    std::vector<int32_t> exp_res({-8, -2, 1, 2, 3, 3, 4, 5, 6, 7});
    for (uint32_t i = 0; i < exp_res.size(); ++i)
    {
        ASSERT_EQ(exp_res[i], arr[i]);
    }
}

TEST(parallel_sort, stress) 
{
    uint32_t max_size = 10;
    uint32_t max_block_size = 10;
    uint32_t tests_count = 10;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, max_size);
    std::uniform_int_distribution<uint32_t> block_size_distribution(2, max_block_size);
    std::uniform_int_distribution<int32_t> elements_distribution(-100, 100);

    for (uint32_t i = 0; i < tests_count; ++i)
    {
        std::cout << "TEST#" << i << std::endl;
        uint32_t cur_size = size_distribution(generator);
        uint32_t cur_block_size = block_size_distribution(generator);

        raw_array<int32_t> arr(cur_size);
        std::vector<int32_t> v(cur_size);
        for (uint32_t j = 0; j < cur_size; ++j)
        {
            int32_t x = elements_distribution(generator);
            arr[j] = x;
            v[j] = x;
        }
        sort_parallel<int32_t>(arr, cur_block_size);
        std::sort(v.begin(), v.end());
        for (uint32_t j = 0; j < cur_size; ++j)
        {
            ASSERT_EQ(arr[j], v[j]);
        }
    }
}