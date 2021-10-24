#include <gtest/gtest.h>
#include "filter_parallel.h"
#include <cstdint>
#include <random>
#include <vector>
#include <functional>
#include <cassert>
#include "constants.h"

bool is_even(int32_t const& x)
{
    return x % 2 == 0;
}

template <typename T>
std::vector<T> filter_sequential(raw_array<T> const& vals, std::function<bool(T const&)> pred)
{
    std::vector<T> res;
    for (uint32_t i = 0; i < vals.size(); ++i)
    {
        if (pred(vals[i]))
        {
            res.push_back(vals[i]);
        }
    }
    return res;
}

TEST(parallel_filter, simple)
{
    raw_array<int32_t> arr(1000);
    for (uint32_t i = 0; i < arr.size(); ++i)
    {
        arr[i] = i;
    }
    raw_array<int32_t> res = filter_parallel<int32_t>(arr, &is_even, 10);
    ASSERT_EQ(500, res.size());
    for (uint32_t i = 0; i < res.size(); ++i)
    {
        ASSERT_EQ(i * 2, res[i]);
    }
}

TEST(parallel_filter, empty_array) 
{
    raw_array<int32_t> arr(0);
    raw_array<int32_t> res = filter_parallel<int32_t>(arr, &is_even, 10);
    ASSERT_EQ(nullptr, res.get_raw_ptr());
    ASSERT_EQ(0, res.size());
}

void stress_filter(
    std::default_random_engine& generator,
    std::function<std::function<bool(int32_t const&)>()> pred_gen,
    int32_t max_abs_elem)
{
    uint32_t max_size = 100000;
    uint32_t max_blocks = 160;
    uint32_t max_divider = 10000;

    std::uniform_int_distribution<uint32_t> size_distribution(1, max_size);
    std::uniform_int_distribution<uint32_t> blocks_distribution(1, max_blocks);
    std::uniform_int_distribution<uint32_t> dividers_distribution(1, max_divider);
    std::uniform_int_distribution<int32_t> elements_distribution(-max_abs_elem, max_abs_elem);

    for (uint32_t i = 0; i < TESTS_COUNT; ++i)
    {
        uint32_t cur_size = size_distribution(generator);
        uint32_t cur_blocks = blocks_distribution(generator);
        uint32_t cur_divider = dividers_distribution(generator);

        raw_array<int32_t> arr(cur_size);
        for (uint32_t j = 0; j < cur_size; ++j)
        {
            arr[j] = elements_distribution(generator);
        }
        std::function<bool(int32_t const&)> pred = pred_gen();
        raw_array<int32_t> res = filter_parallel<int32_t>(arr, pred, cur_blocks);
        std::vector<int32_t> exp_res = filter_sequential(arr, pred);
        ASSERT_EQ(exp_res.size(), res.size());
        for (uint32_t j = 0; j < res.size(); ++j)
        {
            ASSERT_EQ(res[j], exp_res[j]);
        }
    }
}

TEST(parallel_filter, stress_divisors) 
{
    int32_t max_abs_elem = 1000000;
    int32_t max_divider = 10000;
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> dividers_distribution(1, max_divider);
    std::function<std::function<bool(int32_t const&)>()> pred_gen = [&generator, &dividers_distribution]()
    {
        int32_t cur_divider = dividers_distribution(generator);
        return [cur_divider](int32_t const& x)
        {
            return x % cur_divider == 0;
        };
    };
    stress_filter(generator, pred_gen, max_abs_elem);
}

void stress_partitioner(std::function<bool(int32_t, int32_t)> comp)
{
    int32_t max_abs_elem = 1000000;
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> partitioners_distr(1, max_abs_elem);
    std::function<std::function<bool(int32_t const&)>()> pred_gen = [&comp, &generator, &partitioners_distr]()
    {
        int32_t cur_partitioner = partitioners_distr(generator);
        return [&comp, cur_partitioner](int32_t const& x)
        {
            return comp(x, cur_partitioner);
        };
    };
    stress_filter(generator, pred_gen, max_abs_elem);
}

TEST(parallel_filter, stress_less) 
{
    std::function<bool(int32_t, int32_t)> comp_less = [](int32_t x, int32_t partitioner)
    {
        return x < partitioner;
    };
    stress_partitioner(comp_less);
}

TEST(parallel_filter, stress_equal) 
{
    std::function<bool(int32_t, int32_t)> comp_eq = [](int32_t x, int32_t partitioner)
    {
        return x == partitioner;
    };
    stress_partitioner(comp_eq);
}

TEST(parallel_filter, greater) 
{
    std::function<bool(int32_t, int32_t)> comp_gt = [](int32_t x, int32_t partitioner)
    {
        return x > partitioner;
    };
    stress_partitioner(comp_gt);
}