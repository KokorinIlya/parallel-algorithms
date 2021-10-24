#include "filter_parallel.h"
#include <chrono>
#include <random>
#include <iostream>
#include <cstdint>
#include <vector>
#include <functional>

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

uint64_t measure(std::default_random_engine& generator, std::uniform_int_distribution<int32_t>& elements_distribution,
                uint32_t sz, uint32_t blocks_count, uint32_t reps, int32_t divisor)
{
    uint64_t sum = 0;
    std::function<bool(int32_t const&)> pred = [divisor](int32_t const& x)
    {
        return x % divisor == 0;
    };

    for (uint32_t i = 0; i < reps; ++i)
    {
        raw_array<int32_t> arr(sz);
        for (uint32_t j = 0; j < arr.size(); ++j)
        {
            arr[j] = elements_distribution(generator);
        }
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        if (blocks_count == 0)
        {
           filter_sequential(arr, pred);
        }
        else
        {
            filter_parallel<int32_t>(arr, pred, blocks_count);
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        sum += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    }
    return sum / reps;
}

int main()
{
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(-1000000, 1000000);
    uint32_t sz = 10000000;
    uint32_t reps = 10;

    uint64_t res = measure(generator, elements_distribution, sz, 0, reps, 5);
    std::cout << "Sequential, elapsed " << res << " milliseconds" << std::endl;

    for (uint32_t i = 10; i <= 160; i += 10)
    {
        uint64_t res = measure(generator, elements_distribution, sz, i, reps, 5);
        std::cout << i << " blocks, elapsed " << res << " milliseconds" << std::endl;
    }
    return 0;
}
