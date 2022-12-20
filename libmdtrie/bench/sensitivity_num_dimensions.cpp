#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <iostream>
#include <fstream>
#include "../../librpc/src/TrinityParseFile.h"
#include <random>
#include "common.h"

const int DIMENSIONS = 9;

void run_bench(){

    std::vector<int32_t> found_points;
    md_trie<DIMENSIONS> mdtrie(max_depth, trie_depth, max_tree_node);
    data_point<DIMENSIONS> leaf_point;

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
        for (dimension_t i = 0; i < DIMENSIONS; i++) 
            leaf_point.set_coordinate(i, vect[i]);
        

        start = GetTimestamp();
        mdtrie.insert_trie(&leaf_point, n_points, p_key_to_treeblock_compact);
        TimeStamp latency =  GetTimestamp() - start;
        diff += latency;
        n_points ++;

        if (n_points == TPCH_SIZE)
            break; 
    }    
    infile.close();
    std::cout << "mdtrie storage: " << mdtrie.size(p_key_to_treeblock_compact) << std::endl;

    /** 
        Range Search
    */

    std::string outfile_address = results_folder_addr + "sensitivity_query_selectivity";
    std::ifstream file(TPCH_QUERY_ADDR);
    std::ofstream outfile(outfile_address, std::ios_base::app);

    int i = 0;
    while (i < QUERY_NUM * 10) {

        std::vector<int32_t> found_points;
        data_point<DIMENSIONS> start_range;
        data_point<DIMENSIONS> end_range;

        for (dimension_t i = 0; i < DIMENSIONS; i++){
            start_range.set_coordinate(i, gen_rand(tpch_min_values[i], tpch_max_values[i]));
            end_range.set_coordinate(i, gen_rand(start_range.get_coordinate(i), tpch_max_values[i]));
        }

        for (dimension_t i = 0; i < DIMENSIONS; i++){
            if (i >= 4 && i != 7) {
                start_range.set_coordinate(i, start_range.get_coordinate(i) - 19000000);
                end_range.set_coordinate(i, end_range.get_coordinate(i) - 19000000);
            }
        }
        
        start = GetTimestamp();
        mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points);
        diff = GetTimestamp() - start;

        if (found_points.size() / DIMENSIONS < 1000 || found_points.size() / DIMENSIONS > 2000)
            continue;

        outfile << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / TPCH_DIMENSION << std::endl;
        found_points.clear();
        i += 1;
    }
}

int main() {

    use_tpch_setting(DIMENSIONS);
    run_bench();
}