#include <gtest/gtest.h>
#include "raw_array.h"
#include <cstdint>

struct test_struct
{
    int32_t value;

    test_struct(test_struct&& other) : value(other.value)
    {
        other.value = -1;
    }
};


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

TEST(raw_array, test_moving_values)
{
    raw_array<test_struct> arr(10);
    test_struct x;
    x.value = 10;
    arr[0] = std::move(x);
    ASSERT_EQ(arr[0].value, 10);
    ASSERT_EQ(x.value, -1);
}

