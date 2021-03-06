#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include "scan.h"
#include <cassert>

int32_t scan_exclusive_sequential_inplace(raw_array<int32_t>& x)
{
    if (x.size() == 0)
    {
        return 0;
    }
    int32_t t = x[0];
    x[0] = 0;
    for (uint32_t i = 1; i < x.size(); ++i)
    {
        int32_t next_t = x[i];
        x[i] = x[i - 1] + t;
        t = next_t;
    }
    return x[x.size() - 1] + t;
}

std::pair<raw_array<int32_t>, int32_t> scan_exclusive_sequential(raw_array<int32_t> const& x)
{
    if (x.size() == 0)
    {
        return {raw_array<int32_t>(0), 0};
    }
    raw_array<int32_t> psums(x.size());
    psums[0] = 0;
    for (uint32_t i = 1; i < x.size(); ++i)
    {
        psums[i] = psums[i - 1] + x[i - 1];
    }
    int32_t total_sum = psums[x.size() - 1] + x[x.size() - 1];
    return {std::move(psums), total_sum};
}

std::pair<raw_array<int32_t>, int32_t> scan_exclusive_blocked(raw_array<int32_t> const& x, uint32_t blocks_count)
{
    if (x.size() == 0)
    {
        return {raw_array<int32_t>(0), 0};
    }

    uint32_t elements_per_block = x.size() / blocks_count;
    if (x.size() % blocks_count != 0)
    {
        ++elements_per_block;
    }
    assert(elements_per_block >= 1);

    raw_array<int32_t> psums(x.size());
    raw_array<int32_t> deltas(blocks_count);

    #pragma grainsize 1
    cilk_for (uint32_t i = 0; i < blocks_count; ++i)
    {
        uint32_t left = i * elements_per_block;
        uint32_t right = left + elements_per_block;
        if (right > x.size())
        {
            right = x.size();
        }
        if (left < right)
        {
            psums[left] = 0;
            for (uint32_t j = left + 1; j < right; ++j)
            {
                psums[j] = psums[j - 1] + x[j - 1];
            }
            deltas[i] = psums[right - 1] + x[right - 1];
        }
        else
        {
            assert(right == x.size());
        }
    }

    scan_exclusive_sequential_inplace(deltas);

    #pragma grainsize 1
    cilk_for (uint32_t i = 0; i < blocks_count; ++i)
    {
        uint32_t left = i * elements_per_block;
        uint32_t right = left + elements_per_block;
        if (right > x.size())
        {
            right = x.size();
        }
        for (uint32_t j = left; j < right; ++j)
        {
            psums[j] += deltas[i];
        }
    }
    int32_t total_sum = psums[x.size() - 1] + x[x.size() - 1];
    return {std::move(psums), total_sum};
}