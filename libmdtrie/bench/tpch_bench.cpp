#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <iostream>
#include <fstream>
#include "parser.hpp"

// #define NEW_LATENCY_BENCH
#define TPCH_SIZE 250000000 // 250M

const dimension_t DIMENSION = 9;
// level_t max_depth = 32;
// level_t trie_depth = 6;
// preorder_t max_tree_node = 512;
// point_t points_to_insert = 30000;
point_t points_for_warmup = points_to_insert / 5;

// void flush_vector_to_file(std::vector<TimeStamp> vect, std::string filename){
//     #ifdef NEW_LATENCY_BENCH
//     std::ofstream outFile(filename + "_new");
//     #else
//     std::ofstream outFile(filename);
//     #endif
//     for (const auto &e : vect) outFile << std::to_string(e) << "\n";
// }


void run_bench(){

    std::vector<int32_t> found_points;
    md_trie<DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    data_point<DIMENSION> leaf_point;

    std::ifstream infile("/mntData2/dataset_csv/tpch_dataset.csv");
    std::string line;
    point_t n_points = 0;
    std::vector<TimeStamp> insertion_latency_vect;
    std::vector<TimeStamp> lookup_latency_vect;
    TimeStamp start, diff = 0;

    /**
     * Insertion
     */

    while (std::getline(infile, line))
    {
        std::vector<int32_t> vect = parse_line_tpch(line);
        data_point<DIMENSION> leaf_point;

        for (dimension_t i = 0; i < DIMENSION; i++) {
            leaf_point.set_coordinate(i, vect[i]);
        }

        start = GetTimestamp();
        mdtrie.insert_trie(&leaf_point, n_points, p_key_to_treeblock_compact);
        TimeStamp latency =  GetTimestamp() - start;
        diff += latency;
        n_points ++;

        if (n_points > points_for_warmup && n_points <= points_to_insert)
            insertion_latency_vect.push_back(latency);

        #if defined(NEW_LATENCY_BENCH)
        if (n_points == TPCH_SIZE)
            break; 
        #else
        if (n_points == total_points_count)
            break;
        #endif

        #ifdef NEW_LATENCY_BENCH
        if (n_points > TPCH_SIZE - points_to_insert + points_for_warmup)
            insertion_latency_vect.push_back(latency);
        #else
        if (n_points > points_for_warmup && n_points <= points_to_insert)
            insertion_latency_vect.push_back(latency);
        #endif

        if (n_points % (total_points_count / 5000) == 0)
            std::cout << "n_points: "  << n_points << std::endl;
    }    

    std::cout << "Insertion Latency: " << (float) diff / n_points << std::endl;
    std::cout << "mdtrie storage: " << mdtrie.size(p_key_to_treeblock_compact) << std::endl;
    infile.close();
}

int main() {

    /**
     * Set hyperparameters
     * treeblock_size: maximum number of nodes a treeblock can hode
     * trie_depth: the maximum level of the top-level pointer-based trie structure
     * max_depth: the depth of whole data structure
     * bit_widths: the bit widths of each column, with default start-level all set to 0.
     */

    std::vector<level_t> bit_widths = {8, 32, 16, 24, 32, 32, 32, 32, 32}; // 9 Dimensions;
    std::vector<level_t> start_bits = {0, 0, 8, 16, 0, 0, 0, 0, 0}; // 9 Dimensions;
    
    bit_widths = {8, 32, 16, 24, 20, 20, 20, 32, 20};
    trie_depth = 6;
    max_depth = 32;
    no_dynamic_sizing = true;
    total_points_count = 250000000; 
    bitmap::CompactPtrVector tmp_ptr_vect(total_points_count);
    p_key_to_treeblock_compact = &tmp_ptr_vect;

    create_level_to_num_children(bit_widths, start_bits, max_depth);
    run_bench();
}