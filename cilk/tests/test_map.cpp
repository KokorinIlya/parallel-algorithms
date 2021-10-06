#include <gtest/gtest.h>
#include "map.h"
#include <cstdint>

int32_t inc(int32_t const& x)
{
    return x + 1;
}

TEST(parallel_map, simple_test)
{
    raw_array<int32_t> arr(1000);
    for (uint32_t i = 0; i < arr.get_size(); ++i)
    {
        arr[i] = i;
    }
    raw_array<int32_t> res = map_parallel(arr, &inc, 10);
    ASSERT_EQ(arr.get_size(), res.get_size());
    ASSERT_TRUE(res.is_valid());
    for (uint32_t i = 0; i < res.get_size(); ++i)
    {
        ASSERT_EQ(res[i], inc(arr[i]));
    }
}