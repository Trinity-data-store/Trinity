#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <iostream>
#include <fstream>
#include "../../librpc/src/TrinityParseFIle.h"
#include <random>

#define LATENCY_BENCH
#define NEW_LATENCY_BENCH
#define TPCH_SIZE 250000000 // 250M

const dimension_t DIMENSION = 9;
level_t max_depth = 32;
level_t trie_depth = 6;
preorder_t max_tree_node = 512;

void flush_vector_to_file(std::vector<TimeStamp> vect, std::string filename){
    #ifdef NEW_LATENCY_BENCH
    std::ofstream outFile(filename + "_new");
    #else
    std::ofstream outFile(filename);
    #endif
    for (const auto &e : vect) outFile << std::to_string(e) << "\n";
}

int gen_rand(int start, int end) {
    return start + ( std::rand() % ( end - start + 1 ) );
}

void run_bench(){

    std::vector<int32_t> found_points;
    md_trie<DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    data_point<DIMENSION> leaf_point;

    std::ifstream infile("/mntData2/tpch/data_300/tpch_processed_1B.csv");
    std::string line;
    point_t n_points = 0;
    TimeStamp start, diff = 0;

    /**
     * Insertion
     */

    std::cout << "start insertion" << std::endl;

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

        if (n_points % (TPCH_SIZE / 50) == 0)
            std::cout << "n_points: " << n_points << std::endl;

        if (n_points == TPCH_SIZE)
            break;

        n_points ++;
    }    
    infile.close();

    /** 
        Range Search
    */

    char *infile_address = (char *)"/proj/trinity-PG0/Trinity/queries/tpch/tpch_query_converted";
    char *outfile_address = (char *)"/proj/trinity-PG0/Trinity/results/tpch_trinity_query_selectivity";

    // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
    std::vector<int32_t> max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 58063825, 19980802};
    std::vector<int32_t> min_values = {1, 90000, 0, 0, 19920102, 19920131, 19920103, 81300, 19920101};

    std::ifstream file(infile_address);
    std::ofstream outfile(outfile_address, std::ios_base::app);
    int i = 0;
    while (i < 1000000) {

        std::vector<int32_t> found_points;
        data_point<DIMENSION> start_range;
        data_point<DIMENSION> end_range;

        for (dimension_t i = 0; i < DIMENSION; i++){
            start_range.set_coordinate(i, gen_rand(min_values[i], max_values[i]));
            end_range.set_coordinate(i, gen_rand(start_range.get_coordinate(i), max_values[i]));
        }

        for (dimension_t i = 0; i < DIMENSION; i++){
            if (i >= 4 && i != 7) {
                start_range.set_coordinate(i, start_range.get_coordinate(i) - 19000000);
                end_range.set_coordinate(i, end_range.get_coordinate(i) - 19000000);
            }
        }
        
        start = GetTimestamp();
        mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points);
        diff = GetTimestamp() - start;

        if (!found_points.size())
            continue;

        outfile << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / DIMENSION << std::endl;
        found_points.clear();
        i += 1;
    }
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