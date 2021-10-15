#include <gtest/gtest.h>
#include "map_parallel.h"
#include <cstdint>
#include <random>

int32_t inc(int32_t const& x)
{
    return x + 1;
}

TEST(parallel_map, simple)
{
    raw_array<int32_t> arr(1000);
    for (uint32_t i = 0; i < arr.get_size(); ++i)
    {
        arr[i] = i;
    }
    raw_array<int32_t> res = map_parallel<int32_t, int32_t>(arr, &inc, 10);
    ASSERT_EQ(arr.get_size(), res.get_size());
    for (uint32_t i = 0; i < res.get_size(); ++i)
    {
        ASSERT_EQ(res[i], inc(arr[i]));
    }
}

TEST(parallel_map, empty_array) 
{
    raw_array<int32_t> arr(0);
    raw_array<int32_t> res = map_parallel<int32_t, int32_t>(arr, &inc, 10);
    ASSERT_EQ(nullptr, res.get_raw_ptr());
    ASSERT_EQ(0, res.get_size());
}

TEST(parallel_map, stress) 
{
    uint32_t max_size = 100000;
    uint32_t max_blocks = 20;
    uint32_t tests_count = 200;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, max_size);
    std::uniform_int_distribution<uint32_t> blocks_distribution(1, max_blocks);
    std::uniform_int_distribution<int32_t> elements_distribution(-1000000, 1000000);

    for (uint32_t i = 0; i < tests_count; ++i)
    {
        uint32_t cur_size = size_distribution(generator);
        uint32_t cur_blocks = blocks_distribution(generator);

        raw_array<int32_t> arr(cur_size);
        for (uint32_t j = 0; j < cur_size; ++j)
        {
            arr[i] = elements_distribution(generator);
        }
        raw_array<int32_t> res = map_parallel<int32_t, int32_t>(arr, &inc, cur_blocks);
        ASSERT_EQ(arr.get_size(), res.get_size());
        for (uint32_t i = 0; i < res.get_size(); ++i)
        {
            ASSERT_EQ(res[i], inc(arr[i]));
        }
    }
}