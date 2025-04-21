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

bool check_size = false;

// Template function that accepts a reference to an array of any type and size.
template <typename T>
void printArray(const T* arr, std::size_t N) {
    std::cout << "[";
    for (std::size_t i = 0; i < N; ++i) {
        std::cout << arr[i];
        if (i != N - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

// Function to generate a random integer in the range [min, max]
int random_int(int min, int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

void run_example(std::vector<level_t> bit_widths, std::vector<level_t> start_bits, int total_count, dimension_t width) {

    max_depth = (24 * 10) / width;
    if ((24 * 10) % width != 0) {
        max_depth++;
    }

    create_level_to_num_children(bit_widths, start_bits, max_depth, width);

    // std::cout << "Level to num children (width = " << std::to_string(width) << "): ";
    // printArray<morton_t>(level_to_num_children, max_depth);

    md_trie<10> mdtrie(width, max_depth, trie_depth, max_tree_node);
    // md_trie<10> mdtrie(max_depth, trie_depth, max_tree_node);
    bitmap::CompactPtrVector primary_key_to_treeblock_mapping(total_count);

    // Reserve space in a vector for total_count data_point<10> objects
    std::vector<data_point<10>> points;
    points.reserve(total_count);

    for (int primary_key = 0; primary_key < total_count; primary_key++) {
        // data_point<10> point(width);
        data_point<10> point;
        // Use primary_key as the first coordinate (for lookup correctness checking)
        point.set_coordinate(0, primary_key);
        // For the remaining dimensions, assign random values
        for (dimension_t i = 1; i < 10; ++i) {
            point.set_coordinate(i, random_int(1, (int)1 << 16));
        }
        // Push the generated point into the vector
        points.push_back(point);
    }

    /* ----------- INSERT ----------- */
    TimeStamp start = 0, cumulative = 0;
    for (int primary_key = 0; primary_key < total_count; primary_key++) {
        // Insert each point from the vector into the trie.
        start = GetTimestamp();
        mdtrie.insert_trie(&points[primary_key], primary_key, &primary_key_to_treeblock_mapping);
        cumulative += GetTimestamp() - start;
    }
    std::cout << "Insertion Latency: " << (float)cumulative / total_count << " us" << std::endl;
    if (check_size)
      std::cout << "Size after insertion: " << mdtrie.size(&primary_key_to_treeblock_mapping) << std::endl;

    /* ---------- LOOKUP ------------ */
    for (int primary_key = 0; primary_key < total_count; primary_key++)
    {
        data_point<10> *pt = mdtrie.lookup_trie(primary_key, &primary_key_to_treeblock_mapping);
        for (int i = 0; i < 10; i++) {
            if ((int) pt->get_coordinate(i) != (int) points[primary_key].get_coordinate(i)) {
                std::cerr << "Wrong point retrieved!" << std::endl;
                break;
            }
        }
    }

    /* ---------- RANGE QUERY ------------ */
    cumulative = 0;
    int num_queries = 10;
    for (int c = 0; c < num_queries; c++)
    {
        // data_point<10> start_range(width);
        // data_point<10> end_range(width);
        data_point<10> start_range;
        data_point<10> end_range;
        std::vector<int> found_points;
        for (dimension_t i = 0; i < 10; i++)
        {
            start_range.set_coordinate(i, 0);
            end_range.set_coordinate(i, (int)1 << 16);
        }

        start = GetTimestamp();
        mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points);
        // Coordinates are flattened into one vector.
        if ((int)(found_points.size() / 10) != total_count)
        {
            std::cerr << "Wrong number of points found! ("
                      << std::to_string(found_points.size() / 10) << ")" << std::endl;
        }
        cumulative += GetTimestamp() - start;
    }
    std::cout << "Query Latency: " << (float)cumulative / num_queries << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
    int total_count = 100;
    trie_depth = 6;
    no_dynamic_sizing = true;

    // defaults
    max_tree_node = 512;
    optimization_code = OPTIMIZATION_SM;

    int arg;

    while ((arg = getopt(argc, argv, "o:t:s:")) != -1)
    switch (arg)
    {
    case 't':
      max_tree_node = atoi(optarg);
      break;
    case 'o':
      optimization = std::string(optarg);
      if (optimization == "SM")
        optimization_code = OPTIMIZATION_SM;
      if (optimization == "B")
        optimization_code = OPTIMIZATION_B;
      if (optimization == "CN")
        optimization_code = OPTIMIZATION_CN;
      if (optimization == "GM")
        optimization_code = OPTIMIZATION_GM;
      break;
    case 's':
      check_size = true;
      break;
    default:
      abort();
    }

    /* ---------- Initialization ------------ */
    std::vector<level_t> bit_widths = {
        24, 24, 24, 24, 24, 24, 24, 24, 24, 24};
    std::vector<level_t> start_bits = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    run_example(bit_widths, start_bits, total_count, 4);
    run_example(bit_widths, start_bits, total_count, 6);
    run_example(bit_widths, start_bits, total_count, 8);
    run_example(bit_widths, start_bits, total_count, 10);
    run_example(bit_widths, start_bits, total_count, 16);

    return 0;
}