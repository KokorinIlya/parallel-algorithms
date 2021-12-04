#pragma once 

#include "parray.hpp"
#include "datapar.hpp"
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <queue>

template <typename T>
T calc_sum_parallel(pasl::pctl::parray<T> const& arr) 
{
	return pasl::pctl::sum(arr.begin(), arr.end());
}

inline std::vector<int64_t> bfs_sequential(
    uint64_t nodes_count, uint64_t start_node,
    std::unordered_map<uint64_t, std::vector<uint64_t>> const& edges)
{
    assert(0 <= start_node && start_node < nodes_count);
    std::vector<int64_t> result(nodes_count, -1);
    result[start_node] = 0;

    std::queue<uint64_t> q;
    q.push(start_node);

    while (!q.empty())
    {
        uint64_t from_node = q.front();
        q.pop();
        assert(0 <= from_node && from_node < nodes_count);
        assert(result[from_node] >= 0);

        auto it = edges.find(from_node);
        if (it == edges.end())
        {
            continue;
        }
        for (uint64_t to_node : it->second)
        {
            if (result[to_node] == -1)
            {
                result[to_node] = result[from_node] + 1;
                q.push(to_node);
            }
            else
            {
                assert(result[to_node] >= 0);
            }
        }
    }

    return result;
}