#include <gtest/gtest.h>
#include "raw_array.h"
#include <cstdint>

struct test_struct
{
    int32_t value;

    test_struct() {}

    test_struct(test_struct&& other) : value(other.value)
    {
        other.value = -1;
    }

    test_struct& operator=(test_struct&& other)
    {
        this->value = other.value;
        other.value = -1;
        return *this;
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

TEST(raw_array, moving_values)
{
    raw_array<test_struct> arr(10);
    test_struct x;
    x.value = 10;
    arr[0] = std::move(x);
    ASSERT_EQ(arr[0].value, 10);
    ASSERT_EQ(x.value, -1);
}

void write_to_array_copy(moving_array<int32_t> arr)
{
    arr[5] = 100;
}

TEST(raw_array, moving_array)
{
    raw_array<int32_t> arr(10);
    arr[5] = 15;
    write_to_array_copy(arr);
    ASSERT_EQ(arr[5], 15);
}

void write_to_array_ref(moving_array<int32_t>& arr)
{
    arr[5] = 100;
}

TEST(raw_array, moving_array)
{
    raw_array<int32_t> arr(10);
    arr[5] = 15;
    write_to_array_copy(arr);
    ASSERT_EQ(arr[5], 100);
}

void write_to_array_move(moving_array<int32_t>&& arr)
{
    arr[5] = 100;
}

TEST(raw_array, copy_array)
{
    raw_array<int32_t> arr(10);
    arr[5] = 15;
    write_to_array_move(arr);
    ASSERT_FALSE(arr.is_valid());
}

