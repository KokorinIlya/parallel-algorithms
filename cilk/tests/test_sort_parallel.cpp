#include <gtest/gtest.h>
#include "constants.h"
#include "sort.h"
#include <cstdint>
#include <random>
#include <vector>
#include <algorithm>
#include <functional>

void test_simple(bool parallel)
{
    std::vector<int32_t> v({1, 3, 3, 7, -2, 5, 2, 4, 6, -8});
    raw_array<int32_t> arr(v.size());
    for (uint32_t i = 0; i < v.size(); ++i)
    {
        arr[i] = v[i];
    }

    if (parallel)
    {
        sort_parallel<int32_t>(arr, 3);
    }
    else
    {
        sort_sequential<int32_t>(arr);
    }
    
    std::vector<int32_t> exp_res({-8, -2, 1, 2, 3, 3, 4, 5, 6, 7});
    for (uint32_t i = 0; i < exp_res.size(); ++i)
    {
        ASSERT_EQ(exp_res[i], arr[i]);
    }
}

TEST(parallel_sort, simple)
{
    test_simple(true);
}

TEST(parallel_sort_filter_seq, simple)
{
    std::vector<int32_t> arr({1, 3, 3, 7, -2, 5, 2, 4, 6, -8});
    sort_parallel_filter_seq<int32_t>(arr, 3);    
    std::vector<int32_t> exp_res({-8, -2, 1, 2, 3, 3, 4, 5, 6, 7});
    ASSERT_EQ(exp_res, arr);
}

TEST(sequential_sort, simple)
{
    test_simple(false);
}

void test_sort(bool parallel)
{
    uint32_t max_size = 100000;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, max_size);
    std::uniform_int_distribution<uint32_t> block_size_distribution(20, 100);
    std::uniform_int_distribution<int32_t> elements_distribution(-1000000, 1000000);

    for (uint32_t i = 0; i < TESTS_COUNT; ++i)
    {
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
        if (parallel)
        {
            sort_parallel<int32_t>(arr, cur_block_size);
        }
        else
        {
            sort_sequential<int32_t>(arr);
        }
        std::sort(v.begin(), v.end());
        for (uint32_t j = 0; j < cur_size; ++j)
        {
            ASSERT_EQ(arr[j], v[j]);
        }
    }
}

TEST(parallel_sort, stress) 
{
    test_sort(true);
}

TEST(sequential_sort, stress) 
{
    test_sort(false);
}