#include <gtest/gtest.h>
#include "sort_parallel.h"
#include <cstdint>
#include <random>
#include <vector>

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
