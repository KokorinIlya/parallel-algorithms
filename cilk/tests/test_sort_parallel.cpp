#include <gtest/gtest.h>
#include "constants.h"
#include "sort.h"
#include <cstdint>
#include <random>
#include <vector>
#include <algorithm>
#include <functional>

template <template <typename, typename ...> typename C>
void test_simple(std::function<void(C<int32_t>&)> sorter)
{
    std::vector<int32_t> v({1, 3, 3, 7, -2, 5, 2, 4, 6, -8});
    C<int32_t> arr(v.size());
    for (uint32_t i = 0; i < v.size(); ++i)
    {
        arr[i] = v[i];
    }

    sorter(arr);

    std::vector<int32_t> exp_res({-8, -2, 1, 2, 3, 3, 4, 5, 6, 7});
    for (uint32_t i = 0; i < exp_res.size(); ++i)
    {
        ASSERT_EQ(exp_res[i], arr[i]);
    }
}

TEST(sort, parallel_simple)
{
    test_simple<raw_array>(
        [](raw_array<int32_t>& arr)
        {
            sort_parallel(arr, 3);
        }
    );
}

TEST(sort, parallel_filter_seq_simple)
{
    test_simple<std::vector>(
        [](std::vector<int32_t>& arr)
        {
            sort_parallel_filter_seq(arr, 3);
        }
    );
}

TEST(sort, parallel_no_filters)
{
    test_simple<raw_array>(
        [](raw_array<int32_t>& arr)
        {
            sort_parallel_no_filters(arr, 3);
        }
    );
}

TEST(sort, sequential_simple)
{
    test_simple<raw_array>(
        [](raw_array<int32_t>& arr)
        {
            sort_sequential(arr);
        }
    );
}

template <template <typename, typename ...> typename C>
void test_stress(std::function<void(C<int32_t>&)> sorter, std::default_random_engine& generator)
{
    uint32_t max_size = 100000;

    std::uniform_int_distribution<uint32_t> size_distribution(1, max_size);
    std::uniform_int_distribution<int32_t> elements_distribution(-1000000, 1000000);

    for (uint32_t i = 0; i < TESTS_COUNT; ++i)
    {
        uint32_t cur_size = size_distribution(generator);

        C<int32_t> arr(cur_size);
        std::vector<int32_t> v(cur_size);
        for (uint32_t j = 0; j < cur_size; ++j)
        {
            int32_t x = elements_distribution(generator);
            arr[j] = x;
            v[j] = x;
        }

        sorter(arr);
        std::sort(v.begin(), v.end());

        for (uint32_t j = 0; j < cur_size; ++j)
        {
            ASSERT_EQ(arr[j], v[j]);
        }
    }
}

TEST(sort, stress_parallel) 
{
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> block_size_distribution(20, 100);
    test_stress<raw_array>(
        [&generator, &block_size_distribution](raw_array<int32_t>& arr)
        {
            uint32_t cur_block_size = block_size_distribution(generator);
            sort_parallel(arr, cur_block_size);
        },
        generator
    );
}

TEST(sort, stress_parallel_filter_seq) 
{
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> block_size_distribution(20, 100);
    test_stress<std::vector>(
        [&generator, &block_size_distribution](std::vector<int32_t>& arr)
        {
            uint32_t cur_block_size = block_size_distribution(generator);
            sort_parallel_filter_seq(arr, cur_block_size);
        },
        generator
    );
}

TEST(sort, stress_parallel_no_filters) 
{
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<uint32_t> block_size_distribution(20, 100);
    test_stress<std::vector>(
        [&generator, &block_size_distribution](std::vector<int32_t>& arr)
        {
            uint32_t cur_block_size = block_size_distribution(generator);
            sort_parallel_no_filters(arr, cur_block_size);
        },
        generator
    );
}

TEST(sort, stress_sequential) 
{
    std::default_random_engine generator(time(nullptr));
    test_stress<raw_array>(
        [](raw_array<int32_t>& arr)
        {
            sort_sequential(arr);
        },
        generator
    );
}