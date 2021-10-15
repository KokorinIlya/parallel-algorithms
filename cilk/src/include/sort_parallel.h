#ifndef SORT_PARALLEL_H
#define SORT_PARALLEL_H

#include "raw_array.h"
#include "map_parallel.h"
#include "filter_parallel.h"
#include "scan.h"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <functional>
#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <iostream>

template <typename T>
void copy_parallel(raw_array<T> const& src, raw_array<T>& dst, uint32_t start_idx, uint32_t blocks_count)
{
    assert(src.get_size() + start_idx <= dst.get_size());

    if (blocks_count > src.get_size())
    {
        blocks_count = src.get_size();
    }
    uint32_t elements_per_block = src.get_size() / blocks_count;
    if (src.get_size() % blocks_count != 0)
    {
        ++elements_per_block;
    }
    #pragma grainsize 1
    cilk_for (uint32_t i = 0; i < blocks_count; ++i)
    {
        uint32_t left = i * elements_per_block;
        uint32_t right = left + elements_per_block;
        if (right > src.get_size())
        {
            right = src.get_size();
        }
        for (uint32_t j = left; j < right; ++j)
        {
            dst[start_idx + j] = src[j];
        }
    }
}

template <typename T>
void sort_parallel(raw_array<T>& arr, uint32_t seq_sort_threshold, uint32_t blocks_count)
{
    std::cout << "ARR = [";
    for (uint32_t i = 0; i < arr.get_size(); ++i)
    {
        std::cout << arr[i] << " ";
    }
    std::cout << "]" << std::endl;

    if (arr.get_size() == 0)
    {
        return;
    }
    if (arr.get_size() <= seq_sort_threshold)
    {
        T* data_ptr = arr.get_raw_ptr();
        std::sort(data_ptr, data_ptr + arr.get_size());
        return;
    }
    uint32_t partitioner_idx = static_cast<uint32_t>(std::rand()) % arr.get_size();
    T const& partitioner = arr[partitioner_idx];

    std::cout << "PART = " << partitioner << std::endl;

    raw_array<T> le = cilk_spawn filter_parallel<T>(arr, [&partitioner](T const& x) { return x <  partitioner; }, blocks_count);
    raw_array<T> eq = cilk_spawn filter_parallel<T>(arr, [&partitioner](T const& x) { return x == partitioner; }, blocks_count);
    raw_array<T> gt =            filter_parallel<T>(arr, [&partitioner](T const& x) { return x >  partitioner; }, blocks_count);
    cilk_sync;

    std::cout << "LE = [";
    for (uint32_t i = 0; i < le.get_size(); ++i)
    {
        std::cout << le[i] << " ";
    }
    std::cout << "]" << std::endl;
    std::cout << "EQ = [";
    for (uint32_t i = 0; i < eq.get_size(); ++i)
    {
        std::cout << eq[i] << " ";
    }
    std::cout << "]" << std::endl;
    std::cout << "GT = [";
    for (uint32_t i = 0; i < gt.get_size(); ++i)
    {
        std::cout << gt[i] << " ";
    }
    std::cout << "]" << std::endl;

    cilk_spawn copy_parallel(le, arr, 0,                             blocks_count);
    cilk_spawn copy_parallel(eq, arr, le.get_size(),                 blocks_count);
               copy_parallel(gt, arr, le.get_size() + eq.get_size(), blocks_count);
    cilk_sync;
}

#endif