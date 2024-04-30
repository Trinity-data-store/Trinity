#include "benchmark.hpp"
#include "common.hpp"
#include "parser.hpp"
#include "trie.h"
#include <climits>
#include <fstream>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <random>
#include <unordered_set>

// Function to generate a random integer in the range [min, max]
int random_int(int min, int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

// Function to find the intersection of two vectors
std::vector<int> intersection(const std::vector<int> &v1, const std::vector<int> &v2)
{
    // Create unordered sets from the vectors for faster lookup
    std::unordered_set<int> set1(v1.begin(), v1.end());
    std::unordered_set<int> set2(v2.begin(), v2.end());

    // Initialize a vector to store the intersection
    std::vector<int> intersect;

    // Iterate through set1 and check if each element is present in set2
    for (int num : set1)
    {
        if (set2.find(num) != set2.end())
        {
            intersect.push_back(num);
        }
    }

    return intersect;
}

class MdTries
{
public:
    MdTries(dimension_t num_dimensions, std::vector<md_trie<10>> *mdtries,
            std::vector<bitmap::CompactPtrVector *> *ptr_vectors)
    {
        _num_dimensions = num_dimensions;
        _mdtries = mdtries;
        _ptr_vectors = ptr_vectors;
    }

    void insert(std::vector<int> vect, int p_key)
    {
        for (dimension_t i = 0; i < _num_dimensions; i += 10)
        {
            data_point<10> p;
            for (dimension_t j = i; j < i + 10; j++)
            {
                p.set_coordinate(j - i, vect[j]);
            }
            (*_mdtries)[i / 10].insert_trie(&p, p_key, (*_ptr_vectors)[i / 10]);
        }
    }

    std::vector<int> lookup(int p_key)
    {
        std::vector<int> return_point;
        for (dimension_t i = 0; i < _num_dimensions; i += 10)
        {
            std::vector<morton_t> node_path_from_primary(max_depth + 1);
            tree_block<10> *t_ptr =
                (tree_block<10> *)((*_ptr_vectors)[i / 10]->At(p_key));
            morton_t parent_symbol_from_primary =
                t_ptr->get_node_path_primary_key(p_key, node_path_from_primary);
            node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

            data_point<10> *pt = t_ptr->node_path_to_coordinates(node_path_from_primary, 10);
            for (dimension_t j = 0; j < 10; j++)
            {
                return_point.push_back(pt->get_coordinate(j));
            }
        }
        return return_point;
    }

    std::vector<int32_t> range_search(std::vector<int32_t> from_range, std::vector<int32_t> to_range)
    {
        std::vector<int> global_found_points = {};
        for (dimension_t i = 0; i < _num_dimensions; i += 10)
        {
            data_point<10> start;
            data_point<10> to;
            std::vector<int> found_points;
            for (dimension_t j = i; j < i + 10; j++)
            {
                start.set_coordinate(j - i, from_range[j]);
                to.set_coordinate(j - i, to_range[j]);
            }
            (*_mdtries)[i / 10].range_search_trie(
                &start, &to, (*_mdtries)[i / 10].root(), 0, found_points);

            if (i == 0)
            {
                global_found_points = found_points;
            }
            else
            {
                global_found_points.insert(global_found_points.end(), found_points.begin(), found_points.end());
            }
        }
        return global_found_points;
    }

protected:
    std::vector<md_trie<10>> *_mdtries;
    std::vector<bitmap::CompactPtrVector *> *_ptr_vectors;
    dimension_t _num_dimensions;
};

int main()
{
    dimension_t num_dimensions = 20; // Change this to the desired number of dimensions
    int total_count = 100;

    std::vector<md_trie<10>> mdtries;
    std::vector<bitmap::CompactPtrVector *> ptr_vectors;

    /* ---------------------- */
    std::vector<level_t> bit_widths = {
        8, 32, 16, 24, 20, 20, 20, 32, 20}; // 9 Dimensions;
    std::vector<level_t> start_bits = {
        0, 0, 8, 16, 0, 0, 0, 0, 0}; // 9 Dimensions;
    std::vector<level_t> new_start_bits = start_bits;
    std::vector<level_t> new_bit_widths = bit_widths;
    for (dimension_t d = start_bits.size(); d < num_dimensions; d++)
    {
        new_start_bits.push_back(start_bits[d % start_bits.size()]);
        new_bit_widths.push_back(bit_widths[d] % start_bits.size());
    }
    start_bits = new_start_bits;
    bit_widths = new_bit_widths;
    trie_depth = 6;
    max_depth = 32;
    no_dynamic_sizing = true;
    create_level_to_num_children(bit_widths, start_bits, max_depth);

    for (dimension_t i = 0; i < num_dimensions; i += 10)
    {
        mdtries.push_back(md_trie<10>(max_depth, trie_depth, max_tree_node));
        ptr_vectors.push_back(new bitmap::CompactPtrVector(total_count));
    }
    MdTries m = MdTries(num_dimensions, &mdtries, &ptr_vectors);

    /* ----------- INSERT ----------- */
    TimeStamp start = 0, cumulative = 0;

    for (int c = 0; c < total_count; c++)
    {
        std::vector<int> vec;
        for (dimension_t i = 0; i < num_dimensions; ++i)
        {
            vec.push_back(random_int(1, 255));
        }
        start = GetTimestamp();
        m.insert(vec, c);
        cumulative += GetTimestamp() - start;
    }
    std::cout << "Insertion Latency: " << (float)cumulative / total_count << std::endl;

    /* ---------- LOOKUP ------------ */
    cumulative = 0;
    for (int c = 0; c < total_count; c++)
    {
        start = GetTimestamp();
        std::vector<int> vec = m.lookup(c);
        cumulative += GetTimestamp() - start;
    }
    std::cout << "Lookup Latency: " << (float)cumulative / total_count << std::endl;

    /* ---------- RANGE QUERY ------------ */
    cumulative = 0;
    int num_queries = 3;
    for (int c = 0; c < num_queries; c++)
    {
        std::vector<int> low;
        std::vector<int> high;
        std::vector<int> ret;
        // Generate random integers for the vector
        for (dimension_t i = 0; i < num_dimensions; ++i)
        {
            low.push_back(0);
            high.push_back(255);
        }
        start = GetTimestamp();
        ret = m.range_search(low, high);
        std::cout << "found points: " << ret.size() / num_dimensions << std::endl;
        cumulative += GetTimestamp() - start;
    }
    std::cout << "Query Latency: " << (float)cumulative / num_queries << std::endl;

    return 0;
}