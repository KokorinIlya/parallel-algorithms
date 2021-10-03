#include <gtest/gtest.h>
#include "prefix_sum.h"
#include <vector>
#include <random>

TEST(sequential_prefix_sum, simple) 
{
    std::vector<int32_t> x({1, 3, 3, 7, -2, 5});
    std::vector<int32_t> res(x.size());
    calc_sequential(x, res);
    std::vector<int32_t> exp_res({1, 4, 7, 14, 12, 17});
    ASSERT_EQ(exp_res, res);
}

TEST(parallel_prefix_sum, simple) 
{
    std::vector<int32_t> x({1, 3, 3, 7, -2, 5, 2, 4, 6, -8});
    std::vector<int32_t> res(x.size());
    calc_parallel(x, 3, res);
    std::vector<int32_t> exp_res({1, 4, 7, 14, 12, 17, 19, 23, 29, 21});
    ASSERT_EQ(exp_res, res);
}

TEST(parallel_prefix_sum, stress) 
{
    uint32_t max_size = 100000;
    uint32_t max_threads = 16;
    uint32_t tests_count = 200;

    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> size_distribution(1, max_size);
    std::uniform_int_distribution<uint32_t> threads_distribution(1, max_threads);
    std::uniform_int_distribution<int32_t> elements_distribution(-1000, 1000);

    for (uint32_t i = 0; i < tests_count; ++i)
    {
        uint32_t cur_size = size_distribution(generator);
        uint32_t cur_threads = threads_distribution(generator);

        std::vector<int32_t> v;
        v.reserve(cur_size);
        for (uint32_t j = 0; j < cur_size; ++j)
        {
            v.push_back(elements_distribution(generator));
        }

        std::vector<int32_t> res(v.size());
        std::vector<int32_t> exp_res(v.size());
        calc_parallel(v, cur_threads, res);
        calc_sequential(v, exp_res);
        ASSERT_EQ(exp_res, res);
    }
}
