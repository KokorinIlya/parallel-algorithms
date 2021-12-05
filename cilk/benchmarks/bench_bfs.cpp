#include "bfs.h"
#include "graph_builder.h"
#include "parray.hpp"
#include <chrono>
#include <iostream>
#include <vector>
#include <functional>
#include <array>

template <template <typename, typename ...> typename C, std::size_t DIM>
uint64_t measure(
    std::array<uint64_t, DIM> const& dimensions, uint32_t reps,
    std::function<C<int64_t>(
        uint64_t, uint64_t, 
        std::unordered_map<uint64_t, std::vector<uint64_t>> const&
    )> const& bfs_fun)
{
    uint64_t nodes_count = calc_nodes_count(dimensions);
    auto edges = build_graph(dimensions);

    uint64_t sum = 0;
    for (uint32_t i = 0; i < reps; ++i)
    {
        std::cout << "Repetition " << i << std::endl;
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        bfs_fun(nodes_count, 0, edges);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        sum += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    }
    return sum / reps;
}

int main()
{
    std::array<uint64_t, 3> dims = {100, 100, 100};
    uint32_t reps = 5;

    std::cout << "Measuring sequential BFS" << std::endl;
    uint64_t seq_res = measure<std::vector, 3>(dims, reps, bfs_sequential);
    std::cout << "Sequential, elapsed " << seq_res << " milliseconds" << std::endl;

    std::cout << "Measuring parallel + CAS BFS" << std::endl;
    uint64_t cas_res = measure<pasl::pctl::parray, 3>(dims, reps, bfs_cas);
    std::cout << "Parallel + CAS, elapsed " << cas_res << " milliseconds" << std::endl;

    return 0;
}