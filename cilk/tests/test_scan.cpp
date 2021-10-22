#include <gtest/gtest.h>
#include "scan.h"
#include "raw_array.h"
#include <random>
#include <vector>

TEST(sequential_scan, simple) 
{
    std::vector<int32_t> v({1, 3, 3, 7, -2, 5});
    raw_array<int32_t> x(v.size());
    for (uint32_t i = 0; i < v.size(); ++i)
    {
        x[i] = v[i];
    }
    auto [psums, total_sum] = scan_exclusive_sequential(x);
    std::vector<int32_t> exp_res({0, 1, 4, 7, 14, 12});
    ASSERT_EQ(17, total_sum);
    ASSERT_EQ(exp_res.size(), psums.get_size());
    for (uint32_t i = 0; i < exp_res.size(); ++i)
    {
        ASSERT_EQ(exp_res[i], psums[i]);
    }
}

TEST(blocked_scan, simple) 
{
    std::vector<int32_t> v({1, 3, 3, 7, -2, 5, 2, 4, 6, -8});
    raw_array<int32_t> x(v.size());
    for (uint32_t i = 0; i < v.size(); ++i)
    {
        x[i] = v[i];
    }
    auto [psums, total_sum] = scan_exclusive_blocked(x, 3);
    std::vector<int32_t> exp_res({0, 1, 4, 7, 14, 12, 17, 19, 23, 29});
    ASSERT_EQ(21, total_sum);
    ASSERT_EQ(exp_res.size(), psums.get_size());
    for (uint32_t i = 0; i < exp_res.size(); ++i)
    {
        ASSERT_EQ(exp_res[i], psums[i]);
    }
}

TEST(blocked_scan, empty_array) 
{
    raw_array<int32_t> x(0);
    auto [psums, total_sum] = scan_exclusive_blocked(x, 3);
    ASSERT_EQ(0, total_sum);
    ASSERT_EQ(nullptr, psums.get_raw_ptr());
    ASSERT_EQ(0, psums.get_size());
}

TEST(blocked_scan, stress) 
{
    uint32_t max_size = 100000;
    uint32_t max_blocks = 20;
    uint32_t tests_count = 2000;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, max_size);
    std::uniform_int_distribution<uint32_t> blocks_distribution(1, max_blocks);
    std::uniform_int_distribution<int32_t> elements_distribution(-1000, 1000);

    for (uint32_t i = 0; i < tests_count; ++i)
    {
        uint32_t cur_size = size_distribution(generator);
        uint32_t cur_blocks = blocks_distribution(generator);

        raw_array<int32_t> x(cur_size);
        for (uint32_t j = 0; j < cur_size; ++j)
        {
            x[j] = elements_distribution(generator);
        }

        auto [psums_expected, total_sum_expected] = scan_exclusive_sequential(x);
        auto [psums, total_sum] = scan_exclusive_blocked(x, cur_blocks);
        ASSERT_EQ(psums_expected.get_size(), psums.get_size());
        ASSERT_EQ(cur_size, psums.get_size());
        for (uint32_t j = 0; j < cur_size; ++j)
        {
            ASSERT_EQ(psums_expected[j], psums[j]);
        }
    }
}
