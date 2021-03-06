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
    for (uint32_t i = 0; i < arr.size(); ++i)
    {
        arr[i] = i + 10;
    }
    for (uint32_t i = 0; i < arr.size(); ++i)
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

void write_to_array_copy(raw_array<int32_t> arr)
{
    arr[5] = 100;
}

TEST(raw_array, copying_array)
{
    raw_array<int32_t> arr(10);
    arr[5] = 15;
    write_to_array_copy(arr);
    ASSERT_EQ(arr[5], 15);
}

void write_to_array_ref(raw_array<int32_t>& arr)
{
    arr[5] = 100;
}

TEST(raw_array, ref_to_array)
{
    raw_array<int32_t> arr(10);
    arr[5] = 15;
    write_to_array_ref(arr);
    ASSERT_EQ(arr[5], 100);
}

TEST(raw_array, move_array)
{
    raw_array<int32_t> arr(10);
    arr[5] = 15;
    raw_array<int32_t> arr_moved(std::move(arr));
    ASSERT_EQ(arr.get_raw_ptr(), nullptr);
    ASSERT_TRUE(arr_moved.get_raw_ptr() != nullptr);
    ASSERT_EQ(arr_moved[5], 15);
}

TEST(raw_array, create_empty_array)
{
    raw_array<int32_t> arr(0);
    ASSERT_EQ(arr.get_raw_ptr(), nullptr);
}

