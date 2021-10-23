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
#include <algorithm>
#include <iostream>
#include <ctime>
#include <string>
#include <vector>
#include <random>

template <typename T>
uint32_t partition(raw_array<T>& arr, uint32_t left, uint32_t right, std::default_random_engine& generator)
{
    assert(0 <= left && left < right && right < arr.get_size());
    std::uniform_int_distribution<uint32_t> p_idx_distribution(left, right);
    uint32_t partitioner_idx = p_idx_distribution(generator);
    assert(left <= partitioner_idx && partitioner_idx <= right);
    T partitioner = arr[partitioner_idx];

    uint32_t i = left;
    uint32_t j = right;
    while (i <= j)
    {
        while(arr[i] < partitioner)
        {
            ++i;
        }
        while(arr[j] > partitioner)
        {
            --j;
        }
        if (i >= j)
        {
            break;
        }
        std::swap(arr[i], arr[j]);
        ++i;
        --j;
    }
    return j;
}

template <typename T>
void do_sort_sequential(raw_array<T>& arr, uint32_t left, uint32_t right, std::default_random_engine& generator)
{
    if (left >= right)
    {
        return;
    }
    assert(0 <= left && left < right && right < arr.get_size());
    uint32_t p_idx = partition(arr, left, right, generator);
    do_sort_sequential(arr, left,      p_idx, generator);
    do_sort_sequential(arr, p_idx + 1, right, generator);
}

template <typename T>
void sort_sequential(raw_array<T>& arr)
{
    if (arr.get_size() <= 1)
    {
        return;
    }
    std::default_random_engine generator(time(nullptr));
    do_sort_sequential(arr, 0, arr.get_size() - 1, generator);
}

template <typename T>
void copy_parallel(raw_array<T> const& src, raw_array<T>& dst, uint32_t start_idx, uint32_t seq_block_size)
{
    assert(src.get_size() + start_idx <= dst.get_size());
    if (src.get_size() == 0)
    {
        return;
    }

    uint32_t blocks_count = src.get_size() / seq_block_size;
    if (src.get_size() % seq_block_size != 0)
    {
        ++blocks_count;
    }
    
    #pragma grainsize 1
    cilk_for (uint32_t i = 0; i < blocks_count; ++i)
    {
        uint32_t left = i * seq_block_size;
        uint32_t right = left + seq_block_size;
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
void do_sort_parallel(raw_array<T>& arr, uint32_t seq_block_size, std::default_random_engine& generator)
{
    if (arr.get_size() <= 1)
    {
        return;
    }
    if (arr.get_size() <= seq_block_size)
    {
        do_sort_sequential(arr, 0, arr.get_size() - 1, generator);
        return;
    }

    std::uniform_int_distribution<uint32_t> p_idx_distribution(0, arr.get_size() - 1);
    uint32_t partitioner_idx = p_idx_distribution(generator);
    assert(0 <= partitioner_idx && partitioner_idx < arr.get_size());
    T const& partitioner = arr[partitioner_idx];

    uint32_t blocks_count = arr.get_size() / seq_block_size;

    raw_array<T> le = cilk_spawn filter_parallel<T>(
        arr, [&partitioner](T const& x) { return x <  partitioner; }, blocks_count
    );
    raw_array<T> eq = cilk_spawn filter_parallel<T>(
        arr, [&partitioner](T const& x) { return x == partitioner; }, blocks_count
    );
    raw_array<T> gt =            filter_parallel<T>(
        arr, [&partitioner](T const& x) { return x >  partitioner; }, blocks_count
    );
    cilk_sync;

    cilk_spawn do_sort_parallel(le, seq_block_size, generator);
               do_sort_parallel(gt, seq_block_size, generator);
    cilk_sync;

    cilk_spawn copy_parallel(le, arr, 0,                             seq_block_size);
    cilk_spawn copy_parallel(eq, arr, le.get_size(),                 seq_block_size);
               copy_parallel(gt, arr, le.get_size() + eq.get_size(), seq_block_size);
    cilk_sync;
}


template <typename T>
void sort_parallel(raw_array<T>& arr, uint32_t seq_block_size)
{
    std::default_random_engine generator(time(nullptr));
    do_sort_parallel(arr, seq_block_size, generator);
}

#endif