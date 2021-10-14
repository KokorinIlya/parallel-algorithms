#include <gtest/gtest.h>
#include "filter_parallel.h"
#include <cstdint>
#include <random>

bool is_even(int32_t const& x)
{
    return x % 2 == 0;
}

TEST(parallel_filter, simple)
{
    raw_array<int32_t> arr(1000);
    for (uint32_t i = 0; i < arr.get_size(); ++i)
    {
        arr[i] = i;
    }
    raw_array<int32_t> res = filter_parallel<int32_t, int32_t>(arr, &is_even, 10);
    ASSERT_EQ(500, res.get_size());
    ASSERT_TRUE(res.is_valid());
    for (uint32_t i = 0; i < res.get_size(); ++i)
    {
        ASSERT_EQ(i * 2, res[i]);
    }
}
