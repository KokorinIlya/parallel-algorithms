#include <gtest/gtest.h>
#include "raw_array.h"
#include <cstdint>

TEST(raw_array, simple_test)
{
    raw_array<int32_t> arr(10);
    for (uint32_t i = 0; i < arr.get_size(); ++i)
    {
        arr[i] = i + 10;
    }
    for (uint32_t i = 0; i < arr.get_size(); ++i)
    {
        ASSERT_EQ(arr[i], i + 10);
    }
}
