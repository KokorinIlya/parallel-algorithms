#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>

template <std::size_t DIM>
uint64_t coords_to_index(std::array<uint64_t, DIM> const& coords, std::array<uint64_t, DIM> const& dimensions)
{
    static_assert(DIM >= 1);

    std::array<uint64_t, DIM> elems_in_dim;
    elems_in_dim[DIM - 1] = 1;
    for (std::size_t i = DIM - 1; i >= 1; --i)
    {
        elems_in_dim[i - 1] = elems_in_dim[i] * dimensions[i];
    }

    uint64_t result = 0;
    for (std::size_t i = 0; i < DIM; ++i)
    {
        assert(coords[i] < dimensions[i]);
        result += coords[i] * elems_in_dim[i];
    }
    return result;
}

template <std::size_t DIM>
uint64_t calc_nodes_count(std::array<uint64_t, DIM> const& dimensions)
{
    static_assert(DIM >= 1);

    uint64_t result = 1;
    for (std::size_t i = 0; i < DIM; ++i)
    {
        result *= dimensions[i];
    }
    return result;
}

template <std::size_t DIM>
void do_build_graph(
    std::vector<std::vector<uint64_t>>& edges,
    std::array<uint64_t, DIM> const& dimensions, std::array<uint64_t, DIM>& coords, std::size_t cur_coord_idx)
{
    assert(cur_coord_idx <= DIM);
    if (cur_coord_idx == DIM)
    {
        uint64_t cur_idx = coords_to_index(coords, dimensions);
        
        for (std::size_t delta_idx = 0; delta_idx < DIM; ++delta_idx)
        {
            if (coords[delta_idx] + 1 < dimensions[delta_idx])
            {
                coords[delta_idx] += 1;
                uint64_t n_idx = coords_to_index(coords, dimensions);
                edges[cur_idx].push_back(n_idx);
                edges[n_idx].push_back(cur_idx);
                coords[delta_idx] -= 1;
            }
        }
        return;
    }
    else
    {
        for (uint64_t cur_coord = 0; cur_coord < dimensions[cur_coord_idx]; ++cur_coord)
        {
            coords[cur_coord_idx] = cur_coord;
            do_build_graph(edges, dimensions, coords, cur_coord_idx + 1);
        }
    }
}

template <std::size_t DIM>
std::vector<std::vector<uint64_t>> build_graph(std::array<uint64_t, DIM> const& dimensions)
{
    uint64_t nodes_count = calc_nodes_count(dimensions);
    std::vector<std::vector<uint64_t>> edges(nodes_count);
    std::array<uint64_t, DIM> coords;
    do_build_graph(edges, dimensions, coords, 0);
    return edges;
}