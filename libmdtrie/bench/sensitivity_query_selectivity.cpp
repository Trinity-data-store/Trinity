#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <iostream>
#include <fstream>
#include "../../librpc/src/TrinityParseFIle.h"
#include <random>
#include "common.h"

void run_bench(){

    std::vector<int32_t> found_points;
    md_trie<TPCH_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    data_point<TPCH_DIMENSION> leaf_point;

    std::ifstream infile(TPCH_DATA_ADDR);
    point_t n_points = 0;
    TimeStamp start, diff = 0;

    /**
     * Insertion
     */

    std::string line;
    while (std::getline(infile, line))
    {
        std::vector<int32_t> vect = parse_line_tpch(line);
        data_point<TPCH_DIMENSION> leaf_point;

        for (dimension_t i = 0; i < TPCH_DIMENSION; i++) 
            leaf_point.set_coordinate(i, vect[i]);

        start = GetTimestamp();
        mdtrie.insert_trie(&leaf_point, n_points, p_key_to_treeblock_compact);
        TimeStamp latency = GetTimestamp() - start;
        diff += latency;
        n_points ++;

        if (n_points == TPCH_SIZE)
            break; 
    }    
    infile.close();

    /** 
        Range Search
    */

    std::string outfile_address = results_folder_addr + "sensitivity_query_selectivity";
    std::ofstream outfile(outfile_address, std::ios_base::app);

    int i = 0;
    while (i < QUERY_NUM * 10) {

        std::vector<int32_t> found_points;
        data_point<TPCH_DIMENSION> start_range;
        data_point<TPCH_DIMENSION> end_range;

        for (dimension_t i = 0; i < TPCH_DIMENSION; i++){
            start_range.set_coordinate(i, gen_rand(tpch_min_values[i], tpch_max_values[i]));
            end_range.set_coordinate(i, gen_rand(start_range.get_coordinate(i), tpch_max_values[i]));
        }

        for (dimension_t i = 0; i < TPCH_DIMENSION; i++){
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

        outfile << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / TPCH_DIMENSION << std::endl;
        found_points.clear();
        i += 1;
    }
}

int main() {

    use_tpch_setting(TPCH_SIZE);
    run_bench();
}