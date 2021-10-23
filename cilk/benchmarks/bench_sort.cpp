#include "sort.h"
#include "raw_array.h"
#include <chrono>
#include <random>
#include <iostream>
#include <vector>

uint64_t measure(std::default_random_engine& generator, std::uniform_int_distribution<int32_t>& elements_distribution,
                uint32_t sz, uint32_t seq_block_size, uint32_t reps)
{
    uint64_t sum = 0;
    for (uint32_t i = 0; i < reps; ++i)
    {
        raw_array<int32_t> x(sz);
        for (uint32_t j = 0; j < sz; ++j)
        {
            x[j] = elements_distribution(generator);
        }
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        if (seq_block_size == 0)
        {
           sort_sequential(x);
        }
        else
        {
            sort_parallel(x, seq_block_size);
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

    std::vector<uint32_t> block_sizes({100, 1000, 10000, 100000});
    for (uint32_t seq_block_size : block_sizes)
    {
        uint64_t res = measure(generator, elements_distribution, sz, seq_block_size, reps);
        std::cout << seq_block_size << " seq block size, elapsed " << res << " milliseconds" << std::endl;
    }
    return 0;
}
