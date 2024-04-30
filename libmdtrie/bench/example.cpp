#include "benchmark.hpp"
#include "common.hpp"
#include "parser.hpp"
#include "trie.h"
#include <climits>
#include <fstream>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <random>

// Function to generate a random integer in the range [min, max]
int random_int(int min, int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

int main()
{
    dimension_t num_dimensions = 9;
    int total_count = 1000;
    trie_depth = 6;
    max_depth = 32;
    no_dynamic_sizing = true;
    md_trie<9> mdtrie(max_depth, trie_depth, max_tree_node);
    bitmap::CompactPtrVector primary_key_to_treeblock_mapping(total_count);

    /* ---------- Initialization ------------ */
    std::vector<level_t> bit_widths = {
        32, 32, 32, 32, 32, 32, 32, 32, 32};
    std::vector<level_t> start_bits = {
        0, 0, 0, 0, 0, 0, 0, 0, 0};
    create_level_to_num_children(bit_widths, start_bits, max_depth);

    /* ----------- INSERT ----------- */
    TimeStamp start = 0, cumulative = 0;

    for (int primary_key = 0; primary_key < total_count; primary_key++)
    {
        data_point<9> point;
        // For lookup correctness checking.
        point.set_coordinate(0, primary_key);
        for (dimension_t i = 1; i < num_dimensions; ++i)
        {
            point.set_coordinate(i, random_int(1, (int)1 << 16));
        }
        start = GetTimestamp();
        mdtrie.insert_trie(&point, primary_key, &primary_key_to_treeblock_mapping);
        cumulative += GetTimestamp() - start;
    }
    std::cout << "Insertion Latency: " << (float)cumulative / total_count << " us" << std::endl;

    /* ---------- LOOKUP ------------ */
    cumulative = 0;
    for (int primary_key = 0; primary_key < total_count; primary_key++)
    {
        start = GetTimestamp();
        data_point<9> *pt = mdtrie.lookup_trie(primary_key, &primary_key_to_treeblock_mapping);
        if ((int)pt->get_coordinate(0) != primary_key)
        {
            std::cerr << "Wrong point retrieved!" << std::endl;
        }
        cumulative += GetTimestamp() - start;
    }
    std::cout << "Lookup Latency: " << (float)cumulative / total_count << std::endl;

    /* ---------- RANGE QUERY ------------ */
    cumulative = 0;
    int num_queries = 10;
    for (int c = 0; c < num_queries; c++)
    {
        data_point<9> start_range;
        data_point<9> end_range;
        std::vector<int> found_points;
        for (dimension_t i = 0; i < 9; i++)
        {
            start_range.set_coordinate(i, 0);
            end_range.set_coordinate(i, (int)1 << 16);
        }

        start = GetTimestamp();
        mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points);
        // Coordinates are flattened into one vector.
        if ((int)(found_points.size() / num_dimensions) != total_count)
        {
            std::cerr << "Wrong number of points found!" << std::endl;
        }
        cumulative += GetTimestamp() - start;
    }
    std::cout << "Query Latency: " << (float)cumulative / num_queries << std::endl;
    return 0;
}