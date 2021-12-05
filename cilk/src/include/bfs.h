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
    //std::cout << "STARTED " << start_node << std::endl;
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
        /*
        std::cout << "FRONTIER: [";
        for (uint64_t x : cur_frontier)
        {
            std::cout << x << " ";
        }
        std::cout << "]" << std::endl;

        std::cout << "RESULT: [";
        for (int64_t x : result)
        {
            std::cout << x << " ";
        }
        std::cout << "]" << std::endl;

        std::cout << "TAKEN: [";
        for (std::atomic<bool> const& x : taken)
        {
            std::cout << x.load() << " ";
        }
        std::cout << "]" << std::endl;
        */

        assert(cur_frontier.size() > 0);

        pasl::pctl::parray<uint64_t> sizes(
            cur_frontier.size(),
            [&cur_frontier, &edges, &taken, &result](uint64_t idx)
            {
                assert(cur_frontier[idx] >= 0);
                uint64_t cur_node = static_cast<uint64_t>(cur_frontier[idx]);
                assert(taken[cur_node].load() && result[cur_node] >= 0);

                auto it = edges.find(cur_node);
                if (it != edges.end())
                {
                    return static_cast<uint64_t>(it->second.size());
                }
                else
                {
                    return static_cast<uint64_t>(0);
                }
            }
        );

        /*
        std::cout << "SIZES: [";
        for (uint64_t x : sizes)
        {
            std::cout << x << " ";
        }
        std::cout << "]" << std::endl;
        */

        pasl::pctl::parray<uint64_t> pref_sizes = pasl::pctl::scan<
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
        assert(pref_sizes.size() == sizes.size());

        uint64_t new_frontier_size = pref_sizes[pref_sizes.size() - 1] + sizes[sizes.size() - 1];
        pasl::pctl::parray<int64_t> new_frontier(new_frontier_size, static_cast<int64_t>(-1));

        /*
        std::cout << "PREF SIZES: [";
        for (uint64_t x : pref_sizes)
        {
            std::cout << x << " ";
        }
        std::cout << "], TOTAL_SIZE = " << new_frontier_size << std::endl;
        */

        pasl::pctl::parallel_for(
            static_cast<uint64_t>(0), static_cast<uint64_t>(cur_frontier.size()),
            [&edges, &cur_frontier, &taken, &result, &new_frontier, &pref_sizes](uint64_t node_idx)
            {
                assert(cur_frontier[node_idx] >= 0);
                uint64_t from_node = cur_frontier[node_idx];
                assert(taken[from_node].load() && result[from_node] >= 0);
                uint64_t start_idx = pref_sizes[node_idx];

                auto it = edges.find(from_node);
                if (it != edges.end())
                {
                    std::vector<uint64_t> const& to_nodes = it->second;
                    pasl::pctl::parallel_for(
                        static_cast<uint64_t>(0), static_cast<uint64_t>(to_nodes.size()),
                        [&taken, &result, &new_frontier, &to_nodes, start_idx, from_node](uint64_t edge_idx)
                        {
                            uint64_t to_node = to_nodes[edge_idx];
                            bool expected_taken = false;
                            if (taken[to_node].compare_exchange_strong(expected_taken, true))
                            {
                                assert(expected_taken == false);
                                assert(result[to_node] == -1);
                                result[to_node] = result[from_node] + 1;
                                assert(new_frontier[start_idx + edge_idx] == -1);
                                new_frontier[start_idx + edge_idx] = to_node;
                            }
                            else
                            {
                                assert(expected_taken == true);
                            }
                        }
                    );
                }
            }
        );

        cur_frontier = pasl::pctl::filter(
            new_frontier.begin(), new_frontier.end(),
            [](int64_t cur_node)
            {
                assert(cur_node >= -1);
                return cur_node != -1;
            } 
        );
        if (cur_frontier.size() == 0)
        {
            return result;
        }
    }
}