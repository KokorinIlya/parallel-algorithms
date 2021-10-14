#include "map_parallel.h"
#include <chrono>
#include <random>
#include <iostream>
#include <cstdint>

int32_t inc(int32_t const& x)
{
    return x + 1;
}

raw_array<int32_t> map_sequential(raw_array<int32_t> const& from, std::function<int32_t(int32_t const&)> mapper)
{
    raw_array<int32_t> result(from.get_size());
    for (uint32_t i = 0; i < from.get_size(); ++i)
    {
        result[i] = mapper(from[i]);
    }
    return result;
}

uint64_t measure(std::default_random_engine& generator, std::uniform_int_distribution<int32_t>& elements_distribution,
                uint32_t sz, uint32_t blocks_count, uint32_t reps)
{
    uint64_t sum = 0;
    for (uint32_t i = 0; i < reps; ++i)
    {
        raw_array<int32_t> arr(sz);
        for (uint32_t j = 0; j < arr.get_size(); ++j)
        {
            arr[j] = elements_distribution(generator);
        }
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        if (blocks_count == 0)
        {
           map_sequential(arr, &inc);
        }
        else
        {
            map_parallel<int32_t, int32_t>(arr, &inc, blocks_count);
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

    uint64_t res = measure(generator, elements_distribution, sz, 0, reps);
    std::cout << "Sequential, elapsed " << res << " milliseconds" << std::endl;

    for (uint32_t i = 1; i <= 16; ++i)
    {
        uint64_t res = measure(generator, elements_distribution, sz, i * 10, reps);
        std::cout << i * 10 << " blocks, elapsed " << res << " milliseconds" << std::endl;
    }
    return 0;
}
