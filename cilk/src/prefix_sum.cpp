#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <vector>
#include "prefix_sum.h"
#include <cassert>

void calc_sequential(std::vector<int32_t> const& x, std::vector<int32_t>& res)
{
    assert(res.size() == x.size());
    if (x.size() == 0)
    {
        return;
    }
    res[0] = x[0];
    for (uint32_t i = 1; i < x.size(); ++i)
    {
        res[i] = res[i - 1] + x[i];
    }
    return;
}

void calc_parallel(std::vector<int32_t> const& x, uint32_t threads, std::vector<int32_t>& res)
{
    assert(x.size() == res.size());
    if (x.size() == 0)
    {
        return;
    }
    if (threads > x.size())
    {
        threads = x.size();
    }
    uint32_t elements_per_thread = x.size() / threads;
    if (x.size() % threads != 0)
    {
        ++elements_per_thread;
    }

    #pragma grainsize 1
    cilk_for (uint32_t i = 0; i < threads; ++i)
    {
        uint32_t left = i * elements_per_thread;
        uint32_t right = left + elements_per_thread;
        if (right > x.size())
        {
            right = x.size();
        }
        res[left] = x[left];
        for (uint32_t j = left + 1; j < right; ++j)
        {
            res[j] = res[j - 1] + x[j];
        }
    }

    std::vector<int32_t> deltas(threads);
    deltas[0] = 0;
    for (uint32_t i = 1; i < threads; ++i)
    {
        uint32_t cur_left = i * elements_per_thread;
        uint32_t prev_right = cur_left - 1;
        deltas[i] = deltas[i - 1] + res[prev_right];
    }

    #pragma grainsize 1
    cilk_for (uint32_t i = 0; i < threads; ++i)
    {
        uint32_t left = i * elements_per_thread;
        uint32_t right = left + elements_per_thread;

        if (right > x.size())
        {
            right = x.size();
        }
        for (uint32_t j = left; j < right; ++j)
        {
            res[j] += deltas[i];
        }
    }
    return;
}
