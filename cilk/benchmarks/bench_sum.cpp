#include "parray.hpp"
#include "datapar.hpp"
#include <chrono>
#include <iostream>
#include <vector>
#include <functional>
#include <cstdint>
#include <random>

int64_t calc_sum_parallel(pasl::pctl::parray<int64_t> const& arr) 
{
	return pasl::pctl::sum(arr.begin(), arr.end());
}

int64_t calc_sum_sequential(std::vector<int64_t> const& arr) 
{
	int64_t res = 0;
    for (int64_t x : arr)
    {
        res += x;
    }
    return res;
}

template <template <typename, typename ...> typename C>
uint64_t measure(
    std::default_random_engine& generator, std::uniform_int_distribution<int64_t>& elements_distribution,
    uint32_t reps, uint64_t size, std::function<int64_t(C<int64_t> const&)> const& sum_fun)
{
    uint64_t sum = 0;
    for (uint32_t i = 0; i < reps; ++i)
    {
        std::cout << "Repetition#" << i << std::endl;
        C<int64_t> arr(size);
        for (uint64_t j = 0; j < size; ++j)
        {
            arr[j] = elements_distribution(generator);
        }

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        sum_fun(arr);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        sum += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    }
    return sum / reps;
}

int main()
{
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int64_t> elements_distribution(-100'000'000, 100'000'000);
    uint32_t sz = 100'000'000;
    uint32_t reps = 5;

    std::cout << "Measuring sequential sum" << std::endl;
    uint64_t seq_res = measure<std::vector>(generator, elements_distribution, reps, sz, calc_sum_sequential);
    std::cout << "Sequential, elapsed " << seq_res << " milliseconds" << std::endl;

    std::cout << "Measuring parallel sum" << std::endl;
    uint64_t par_res = measure<pasl::pctl::parray>(generator, elements_distribution, reps, sz, calc_sum_parallel);
    std::cout << "Parallel, elapsed " << par_res << " milliseconds" << std::endl;

    return 0;
}