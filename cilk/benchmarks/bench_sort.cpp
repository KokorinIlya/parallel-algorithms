#include "sort.h"
#include "raw_array.h"
#include <chrono>
#include <random>
#include <iostream>
#include <vector>
#include <functional>

template <template <typename, typename ...> typename C>
uint64_t measure(
    std::default_random_engine& generator, std::uniform_int_distribution<int32_t>& elements_distribution,
    uint32_t sz, uint32_t reps, std::function<void(C<int32_t>&)> sorter)
{
    uint64_t sum = 0;
    for (uint32_t i = 0; i < reps; ++i)
    {
        C<int32_t> arr(sz);
        for (uint32_t j = 0; j < sz; ++j)
        {
            arr[j] = elements_distribution(generator);
        }

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        sorter(arr);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        sum += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    }
    return sum / reps;
}

int main()
{
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int32_t> elements_distribution(-100'000'000, 100'000'000);
    uint32_t sz = 100'000'000;
    uint32_t reps = 5;

    uint64_t res = measure<raw_array>(
        generator, elements_distribution, sz, reps,
        [](raw_array<int32_t>& arr)
        {
            sort_sequential(arr);
        }
    );
    std::cout << "Sequential, elapsed " << res << " milliseconds" << std::endl;

    std::vector<uint32_t> block_sizes({10, 100, 1'000, 10'000, 100'000, 1'000'000, 10'000'000, 30'000'000});

    for (uint32_t seq_block_size : block_sizes)
    {
        uint64_t res = measure<raw_array>(
            generator, elements_distribution, sz, reps,
            [seq_block_size](raw_array<int32_t>& arr)
            {
                sort_parallel(arr, seq_block_size);
            }
        );
        std::cout << "Parallel: " << seq_block_size << 
            " seq block size, elapsed " << res << " milliseconds" << std::endl;

        res = measure<std::vector>(
            generator, elements_distribution, sz, reps,
            [seq_block_size](std::vector<int32_t>& arr)
            {
                sort_parallel_filter_seq(arr, seq_block_size);
            }
        );
        std::cout << "Parallel, seq filter: " << seq_block_size << 
            " seq block size, elapsed " << res << " milliseconds" << std::endl;

        res = measure<std::vector>(
            generator, elements_distribution, sz, reps,
            [seq_block_size](std::vector<int32_t>& arr)
            {
                sort_parallel_no_filters(arr, seq_block_size);
            }
        );
        std::cout << "Parallel, no filters: " << seq_block_size << 
            " seq block size, elapsed " << res << " milliseconds" << std::endl;
    }
    return 0;
}
