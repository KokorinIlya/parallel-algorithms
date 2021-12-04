#pragma once

#include "graph_builder.h"
#include <array>
#include <gtest/gtest.h>
#include <cstdint>
#include <unordered_set>
#include "bfs.h"
#include <vector>
#include <functional>

template <std::size_t DIM>
void do_get_all_points(
    std::array<uint64_t, DIM> const& dimensions, 
    std::array<uint64_t, DIM>& coords, std::size_t cur_coord_idx, 
    std::vector<std::array<uint64_t, DIM>>& result)
{
    assert(cur_coord_idx <= DIM);
    if (cur_coord_idx == DIM)
    {
        result.push_back(coords);
        return;
    }
    else
    {
        for (uint64_t cur_coord = 0; cur_coord < dimensions[cur_coord_idx]; ++cur_coord)
        {
            coords[cur_coord_idx] = cur_coord;
            do_get_all_points(dimensions, coords, cur_coord_idx + 1, result);
        }
    }
}

template <std::size_t DIM>
std::vector<std::array<uint64_t, DIM>> get_all_points(std::array<uint64_t, DIM> const& dimensions)
{
    std::vector<std::array<uint64_t, DIM>> result;
    std::array<uint64_t, DIM> coords;
    do_get_all_points(dimensions, coords, 0, result);
    return result;
}

template <std::size_t DIM>
uint64_t get_dist(std::array<uint64_t, DIM> const& pt_a, std::array<uint64_t, DIM> const& pt_b)
{
    uint64_t result = 0;
    for (std::size_t i = 0; i < DIM; ++i)
    {
        if (pt_a[i] > pt_b[i])
        {
            result += (pt_a[i] - pt_b[i]);
        }
        else
        {
            result += (pt_b[i] - pt_a[i]);
        }
    }
    return result;
}

template <template <typename, typename ...> typename C, std::size_t DIM>
void test_bfs_cubic(std::array<uint64_t, DIM> const& dimensions,
                    std::function<C<int64_t>(
                        uint64_t, uint64_t, 
                        std::unordered_map<uint64_t, std::vector<uint64_t>> const&
                    )> bfs_fun)
{
    auto edges = build_graph(dimensions);
    uint64_t nodes_count = calc_nodes_count(dimensions);
    auto all_points = get_all_points(dimensions);
    assert(all_points.size() == nodes_count);

    for (auto const& start_point : all_points)
    {
        uint64_t start_idx = coords_to_index(start_point, dimensions);
        C<int64_t> result = bfs_fun(nodes_count, start_idx, edges);
        assert(result.size() == nodes_count);

        for (uint64_t i = 0; i < nodes_count; ++i)
        {
            ASSERT_TRUE(result[i] >= 0);
            ASSERT_EQ(get_dist(start_point, all_points[i]), result[i]);
        }
    }
}

TEST(sequential_bfs, stress_two_dimensions)
{
    std::array<uint64_t, 2> dims = {10, 20};
    test_bfs_cubic<std::vector, 2>(dims, bfs_sequential);
}

TEST(sequential_bfs, stress_three_dimensions)
{
    std::array<uint64_t, 3> dims = {3, 10, 5};
    test_bfs_cubic<std::vector, 3>(dims, bfs_sequential);
}