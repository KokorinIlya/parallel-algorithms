#pragma once 

#include "parray.hpp"
#include "datapar.hpp"
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <queue>
#include <atomic>
#include <functional>

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

inline pasl::pctl::parray<int64_t> bfs_cas(
    uint64_t nodes_count, uint64_t start_node,
    std::unordered_map<uint64_t, std::vector<uint64_t>> const& edges)
{
    assert(0 <= start_node && start_node < nodes_count);
    pasl::pctl::parray<int64_t> result(nodes_count, static_cast<int64_t>(-1));
    result[start_node] = 0;
    pasl::pctl::parray<std::atomic<bool>> taken(
        nodes_count,
        [](long) -> bool
        {
            return false;
        } 
    );
    taken[start_node].store(true);
    pasl::pctl::parray<int64_t> cur_frontier = {static_cast<int64_t>(start_node)};

    while (true)
    {
        assert(cur_frontier.size() > 0);

        pasl::pctl::parray<uint64_t> sizes(
            cur_frontier.size(),
            [&cur_frontier, &edges, &taken](uint64_t idx)
            {
                uint64_t cur_node = cur_frontier[idx];
                assert(cur_node >= 0 && taken[idx].load());
                auto it = edges.find(cur_node);
                assert(it != edges.end());
                return it->second.size();
            }
        );

        uint64_t last_node_size = sizes[sizes.size() - 1];
        pasl::pctl::scan<
            pasl::pctl::parray<uint64_t>::iterator,
            uint64_t,
            std::function<uint64_t(uint64_t, uint64_t)>
        >(
            sizes.begin(), sizes.end(), 0,
            [](uint64_t x, uint64_t y)
            {
                return x + y;
            },
            pasl::pctl::scan_type::forward_exclusive_scan
        );

        uint64_t new_frontier_size = sizes[sizes.size() - 1] + last_node_size;
        pasl::pctl::parray<int64_t> new_frontier(new_frontier_size, static_cast<int64_t>(-1));

        // TODO

        cur_frontier = pasl::pctl::filter(
            new_frontier.begin(), new_frontier.end(),
            [](int64_t cur_node)
            {
                return cur_node != -1;
            } 
        );
        if (cur_frontier.size() == 0)
        {
            return result;
        }
    }
}