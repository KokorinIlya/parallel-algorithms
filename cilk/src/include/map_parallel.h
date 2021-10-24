#ifndef MAP_PARALLEL_H
#define MAP_PARALLEL_H

#include "raw_array.h"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <functional>
#include <cstdint>

template <typename F, typename T>
raw_array<T> map_parallel(raw_array<F> const& from, std::function<T(F const&)> mapper, uint32_t blocks_count)
{
    if (from.size() == 0)
    {
        return raw_array<T>(0);
    }

    uint32_t elements_per_block = from.size() / blocks_count;
    if (from.size() % blocks_count != 0)
    {
        ++elements_per_block;
    }

    raw_array<T> result(from.size());

    #pragma grainsize 1
    cilk_for (uint32_t i = 0; i < blocks_count; ++i)
    {
        uint32_t left = i * elements_per_block;
        uint32_t right = left + elements_per_block;
        if (right > from.size())
        {
            right = from.size();
        }
        for (uint32_t j = left; j < right; ++j)
        {
            result[j] = mapper(from[j]);
        }
    }
    return result;
}

#endif