#pragma once

#include "graph_builder.h"
#include <array>
#include <gtest/gtest.h>
#include <cstdint>
#include <unordered_set>

TEST(coords_to_index, one_dimensional)
{
    std::array<uint64_t, 1> dims = {10};
    for (uint64_t i = 0; i < dims[0]; ++i)
    {
        uint64_t res = coords_to_index({i}, dims);
        ASSERT_EQ(i, res);
    }
}

TEST(coords_to_index, two_dimensional)
{
    std::array<uint64_t, 2> dims = {10, 20};
    std::unordered_set<uint64_t> indexes;
    for (uint64_t i = 0; i < dims[0]; ++i)
    {
        for (uint64_t j = 0; j < dims[1]; ++j)
        {
            uint64_t res = coords_to_index({i, j}, dims);
            ASSERT_EQ(i * 20 + j, res);
            ASSERT_EQ(indexes.end(), indexes.find(res));
            bool insert_res = indexes.insert(res).second;
            ASSERT_TRUE(insert_res);
            ASSERT_NE(indexes.end(), indexes.find(res));
        }
    }
}

TEST(coords_to_index, three_dimensional)
{
    std::array<uint64_t, 3> dims = {10, 20, 30};
    std::unordered_set<uint64_t> indexes;
    for (uint64_t i = 0; i < dims[0]; ++i)
    {
        for (uint64_t j = 0; j < dims[1]; ++j)
        {
            for (uint64_t k = 0; k < dims[2]; ++k)
            {
                uint64_t res = coords_to_index({i, j, k}, dims);
                ASSERT_EQ(i * 20 * 30 + j * 30 + k, res);
                ASSERT_EQ(indexes.end(), indexes.find(res));
                bool insert_res = indexes.insert(res).second;
                ASSERT_TRUE(insert_res);
                ASSERT_NE(indexes.end(), indexes.find(res));
            }
        }
    }
}

TEST(build_graph, one_dimensional)
{
    std::array<uint64_t, 1> dims = {10};
    auto edges = build_graph(dims);
    ASSERT_EQ(std::vector<uint64_t>{1}, edges[0]);
    for (uint64_t i = 1; i < 9; ++i)
    {
        ASSERT_EQ(std::vector<uint64_t>({i - 1, i + 1}), edges[i]);
    }
    ASSERT_EQ(std::vector<uint64_t>{8}, edges[9]);
}

bool check_vector(std::vector<uint64_t> v, std::unordered_set<uint64_t> expected)
{
    if (v.size() != expected.size())
    {
        return false;
    }
    std::unordered_set<uint64_t> v_s;
    for (uint64_t x : v)
    {
        if (v_s.find(x) != v_s.end())
        {
            return false;
        }
        bool insert_res = v_s.insert(x).second;
        assert(insert_res);
        if (expected.find(x) == expected.end())
        {
            return false;
        }
    }
    return true;
}

TEST(build_graph, two_dimensional)
{
    std::array<uint64_t, 2> dims = {10, 20};
    auto edges = build_graph(dims);
    for (uint64_t i = 0; i < dims[0]; ++i)
    {
        for (uint64_t j = 0; j < dims[1]; ++j)
        {
            std::unordered_set<uint64_t> expected;
            if (i > 0)
            {
                expected.insert(coords_to_index({i - 1, j}, dims));
            }
            if (i + 1 < dims[0])
            {
                expected.insert(coords_to_index({i + 1, j}, dims));
            }
            if (j > 0)
            {
                expected.insert(coords_to_index({i, j - 1}, dims));
            }
            if (j + 1 < dims[1])
            {
                expected.insert(coords_to_index({i, j + 1}, dims));
            }
            check_vector(edges[coords_to_index({i, j}, dims)], expected);
        }
    }
}

TEST(build_graph, three_dimensional)
{
    std::array<uint64_t, 3> dims = {10, 20, 30};
    auto edges = build_graph(dims);
    for (uint64_t i = 0; i < 10; ++i)
    {
        for (uint64_t j = 0; j < 20; ++j)
        {
            for (uint64_t k = 0; k < dims[2]; ++k)
            {
                std::unordered_set<uint64_t> expected;
                if (i > 0)
                {
                    expected.insert(coords_to_index({i - 1, j, k}, dims));
                }
                if (i + 1 < dims[0])
                {
                    expected.insert(coords_to_index({i + 1, j, k}, dims));
                }

                if (j > 0)
                {
                    expected.insert(coords_to_index({i, j - 1, k}, dims));
                }
                if (j + 1 < dims[1])
                {
                    expected.insert(coords_to_index({i, j + 1, k}, dims));
                }

                if (k > 0)
                {
                    expected.insert(coords_to_index({i, j, k - 1}, dims));
                }
                if (k + 1 < dims[2])
                {
                    expected.insert(coords_to_index({i, j, k + 1}, dims));
                }
                check_vector(edges[coords_to_index({i, j, k}, dims)], expected);
            }
        }
    }
}