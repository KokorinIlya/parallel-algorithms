#pragma once

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

/*
Sequential sort
*/

template <typename T, template <typename, typename ...> typename C>
uint32_t partition(C<T>& arr, uint32_t left, uint32_t right, std::default_random_engine& generator)
{
    assert(0 <= left && left < right && right < arr.size());
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

template <typename T, template <typename, typename ...> typename C>
void do_sort_sequential(C<T>& arr, uint32_t left, uint32_t right, std::default_random_engine& generator)
{
    if (left >= right)
    {
        return;
    }
    assert(0 <= left && left < right && right < arr.size());
    uint32_t p_idx = partition(arr, left, right, generator);
    do_sort_sequential(arr, left,      p_idx, generator);
    do_sort_sequential(arr, p_idx + 1, right, generator);
}

template <typename T, template <typename, typename ...> typename C>
void sort_sequential(C<T>& arr)
{
    if (arr.size() <= 1)
    {
        return;
    }
    std::default_random_engine generator(time(nullptr));
    do_sort_sequential(arr, 0, arr.size() - 1, generator);
}

/*
Parallel sort without filtering
*/

template <typename T, template <typename, typename ...> typename C>
void sort_parallel_no_filters(
    C<T>& arr, uint32_t left, uint32_t right, uint32_t seq_block_size,
    std::default_random_engine& generator)
{
    if (left >= right)
    {
        return;
    }
    assert(0 <= left && left < right && right < arr.size());
    uint32_t p_idx = partition(arr, left, right, generator);

    if (right - left + 1 <= seq_block_size)
    {
        sort_parallel_no_filters(arr, left,      p_idx, seq_block_size, generator);
        sort_parallel_no_filters(arr, p_idx + 1, right, seq_block_size, generator);
    }
    else
    {
        cilk_spawn sort_parallel_no_filters(arr, left,      p_idx, seq_block_size, generator);
                   sort_parallel_no_filters(arr, p_idx + 1, right, seq_block_size, generator);
        cilk_sync;
    }
}

template <typename T, template <typename, typename ...> typename C>
void sort_parallel_no_filters(C<T>& arr, uint32_t seq_block_size)
{
    if (arr.size() <= 1)
    {
        return;
    }
    std::default_random_engine generator(time(nullptr));
    sort_parallel_no_filters(arr, 0, arr.size() - 1, seq_block_size, generator);
}

/*
Parallel sort with parallel filters
*/

template <typename T, template <typename, typename ...> typename C>
void copy_parallel(C<T> const& src, C<T>& dst, uint32_t start_idx, uint32_t seq_block_size)
{
    assert(src.size() + start_idx <= dst.size());
    if (src.size() == 0)
    {
        return;
    }

    uint32_t blocks_count = src.size() / seq_block_size;
    if (src.size() % seq_block_size != 0)
    {
        ++blocks_count;
    }
    
    #pragma grainsize 1
    cilk_for (uint32_t i = 0; i < blocks_count; ++i)
    {
        uint32_t left = i * seq_block_size;
        uint32_t right = left + seq_block_size;
        if (right > src.size())
        {
            right = src.size();
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
    if (arr.size() <= 1)
    {
        return;
    }
    if (arr.size() <= seq_block_size)
    {
        do_sort_sequential(arr, 0, arr.size() - 1, generator);
        return;
    }

    std::uniform_int_distribution<uint32_t> p_idx_distribution(0, arr.size() - 1);
    uint32_t partitioner_idx = p_idx_distribution(generator);
    assert(0 <= partitioner_idx && partitioner_idx < arr.size());
    T const& partitioner = arr[partitioner_idx];

    uint32_t blocks_count = arr.size() / seq_block_size;

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

    cilk_spawn copy_parallel(le, arr, 0,                     seq_block_size);
    cilk_spawn copy_parallel(eq, arr, le.size(),             seq_block_size);
               copy_parallel(gt, arr, le.size() + eq.size(), seq_block_size);
    cilk_sync;
}


template <typename T>
void sort_parallel(raw_array<T>& arr, uint32_t seq_block_size)
{
    std::default_random_engine generator(time(nullptr));
    do_sort_parallel(arr, seq_block_size, generator);
}

/*
Parallel sort with sequential filters
*/

template <typename T>
std::vector<T> filter_sequential(std::vector<T> const& vals, std::function<bool(T const&)> pred)
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

template <typename T>
void do_sort_parallel_filter_seq(std::vector<T>& arr, uint32_t seq_block_size, std::default_random_engine& generator)
{
    if (arr.size() <= 1)
    {
        return;
    }
    if (arr.size() <= seq_block_size)
    {
        do_sort_sequential<T, std::vector>(arr, 0, arr.size() - 1, generator);
        return;
    }

    std::uniform_int_distribution<uint32_t> p_idx_distribution(0, arr.size() - 1);
    uint32_t partitioner_idx = p_idx_distribution(generator);
    assert(0 <= partitioner_idx && partitioner_idx < arr.size());
    T const& partitioner = arr[partitioner_idx];

    std::vector<T> le = cilk_spawn filter_sequential<T>(
        arr, [&partitioner](T const& x) { return x <  partitioner; }
    );
    std::vector<T> eq = cilk_spawn filter_sequential<T>(
        arr, [&partitioner](T const& x) { return x == partitioner; }
    );
    std::vector<T> gt =            filter_sequential<T>(
        arr, [&partitioner](T const& x) { return x >  partitioner; }
    );
    cilk_sync;

    cilk_spawn do_sort_parallel_filter_seq(le, seq_block_size, generator);
               do_sort_parallel_filter_seq(gt, seq_block_size, generator);
    cilk_sync;

    cilk_spawn copy_parallel(le, arr, 0,                     seq_block_size);
    cilk_spawn copy_parallel(eq, arr, le.size(),             seq_block_size);
               copy_parallel(gt, arr, le.size() + eq.size(), seq_block_size);
    cilk_sync;
}

template <typename T>
void sort_parallel_filter_seq(std::vector<T>& arr, uint32_t seq_block_size)
{
    std::default_random_engine generator(time(nullptr));
    do_sort_parallel_filter_seq(arr, seq_block_size, generator);
}
