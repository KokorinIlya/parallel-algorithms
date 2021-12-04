#pragma once 

#include "bfs.h"
#include "parray.hpp"
#include <gtest/gtest.h>
#include <cstdint>

TEST(sum, simple)
{
    pasl::pctl::parray<int32_t> arr = {2, 5, 1, 7};
    ASSERT_EQ(15, calc_sum_parallel(arr));
}