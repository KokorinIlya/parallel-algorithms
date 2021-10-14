#include "raw_array.h"
#include "map_parallel.h"
#include "scan.h"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <functional>
#include <cstdint>
#include <cassert>

template <typename T>
raw_array<T> filter_parallel(raw_array<T> const& vals, std::function<bool(T const&)> mapper, uint32_t blocks_count)
{
    assert(vals.is_valid() && vals.get_size() > 0);

    if (blocks_count > vals.get_size())
    {
        blocks_count = vals.get_size();
    }
    uint32_t elements_per_block = vals.get_size() / blocks_count;
    if (vals.get_size() % blocks_count != 0)
    {
        ++elements_per_block;
    }

    raw_array<int32_t> flags = map_parallel<T, int32_t>(
        vals,
        [&mapper](val)
        {
            if (mapper(val))
            {
                return 1
            }
            else
            {
                return 0
            }
        },
        blocks_count
    );
    assert(flags.is_valid() && flags.get_size() == vals.get_size());

    const [idxs, total_elems] = scan_exclusive_blocked(flags, blocks_count);
    assert(idxs.is_valid() && idxs.get_size() == vals.get_size());
    
    raw_array<T> res(total_elems);
    
    #pragma grainsize 1
    cilk_for (uint32_t i = 0; i < blocks_count; ++i)
    {
        uint32_t left = i * elements_per_block;
        uint32_t right = left + elements_per_block;
        if (right > x.get_size())
        {
            right = x.get_size();
        }
        for (uint32_t j = left; j < right; ++j)
        {
            if (flags[j] == 1)
            {
                res[idxs[j]] = vals[j];
            }
        }
    }
    return res;
}
