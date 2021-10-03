#include "prefix_sum.h"
#include <chrono>
#include <random>
#include <iostream>

uint64_t measure(std::default_random_engine& generator, std::uniform_int_distribution<int32_t>& elements_distribution,
                uint32_t sz, uint32_t threads, uint32_t reps)
{
    uint64_t sum = 0;
    for (uint32_t i = 0; i < reps; ++i)
    {
        std::vector<int32_t> v(sz);
        for (uint32_t j = 0; j < v.size(); ++j)
        {
            v[j] = elements_distribution(generator);
        }
        std::vector<int32_t> res(sz);
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        if (threads == 0)
        {
           calc_sequential(v, res);
        }
        else
        {
            calc_parallel(v, 10 * threads, res);
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        sum += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    }
    return sum / reps;
}

int main()
{
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(-1000, 1000);
    uint32_t sz = 10000000;
    uint32_t reps = 10;

    uint64_t res = measure(generator, elements_distribution, sz, 0, reps);
    std::cout << "Sequential, elapsed " << res << " microseconds" << std::endl;

    for (uint32_t i = 1; i <= 16; ++i)
    {
        uint64_t res = measure(generator, elements_distribution, sz, i, reps);
        std::cout << i * 10 << " chunks, elapsed " << res << " microseconds" << std::endl;
    }
    return 0;
}
