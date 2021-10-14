#include <gtest/gtest.h>
#include "filter_parallel.h"
#include <cstdint>
#include <random>
#include <vector>
#include <functional>
#include <cassert>

bool is_even(int32_t const& x)
{
    return x % 2 == 0;
}

template <typename T>
std::vector<T> filter_sequential(raw_array<T> const& vals, std::function<bool(T const&)> pred)
{
    std::vector<T> res;
    assert(vals.is_valid() && vals.get_size() > 0);
    for (uint32_t i = 0; i < vals.get_size(); ++i)
    {
        if (prev(vals[i]))
        {
            res.append(vals[i]);
        }
    }
    return res;
}


TEST(parallel_filter, simple)
{
    raw_array<int32_t> arr(1000);
    for (uint32_t i = 0; i < arr.get_size(); ++i)
    {
        arr[i] = i;
    }
    raw_array<int32_t> res = filter_parallel<int32_t>(arr, &is_even, 10);
    ASSERT_EQ(500, res.get_size());
    ASSERT_TRUE(res.is_valid());
    for (uint32_t i = 0; i < res.get_size(); ++i)
    {
        ASSERT_EQ(i * 2, res[i]);
    }
}

TEST(parallel_filter, stress) 
{
    uint32_t max_size = 100000;
    uint32_t max_blocks = 20;
    uint32_t tests_count = 200;
    uint32_t max_divider = 10000;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, max_size);
    std::uniform_int_distribution<uint32_t> blocks_distribution(1, max_blocks);
    std::uniform_int_distribution<uint32_t> dividers_distribution(1, max_divider);
    std::uniform_int_distribution<int32_t> elements_distribution(-1000000, 1000000);

    for (uint32_t i = 0; i < tests_count; ++i)
    {
        uint32_t cur_size = size_distribution(generator);
        uint32_t cur_blocks = blocks_distribution(generator);
        uint32_t cur_divider = dividers_distribution(generator);

        raw_array<int32_t> arr(cur_size);
        for (uint32_t j = 0; j < cur_size; ++j)
        {
            arr[i] = elements_distribution(generator);
        }
        std::function<bool(int32_t const&)> pred = [cur_divider](int32_t const& x)
        {
            return x % cur_divider == 0;
        };
        raw_array<int32_t> res = filter_parallel<int32_t>(arr, pred, cur_blocks);
        std::vector<int32_t> exp_res = filter_sequential(arr, pred);
        ASSERT_EQ(exp_res.size(), res.get_size());
        ASSERT_TRUE(res.is_valid());
        for (uint32_t i = 0; i < res.get_size(); ++i)
        {
            ASSERT_EQ(res[i], exp_res[i]);
        }
    }
}