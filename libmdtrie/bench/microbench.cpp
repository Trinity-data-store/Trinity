#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <fstream>
#include "parser.hpp"
#include "common.hpp"
#include "benchmark.hpp"

void github_bench(void) {
    
    optimization_code = OPTIMIZATION_SM;
    use_github_setting(GITHUB_DIMENSION, GITHUB_SIZE);
    md_trie<GITHUB_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    MdTrieBench<GITHUB_DIMENSION> bench(&mdtrie);
    p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);

    bench.insert(GITHUB_DATA_ADDR, "github_insert_micro" + identification_string, total_points_count, parse_line_github);
    bench.lookup("github_lookup_micro" + identification_string);
    bench.range_search(GITHUB_QUERY_ADDR, "github_query_micro" + identification_string, get_query_github);
}

void nyc_bench(void) {

    optimization_code = OPTIMIZATION_SM;
    use_nyc_setting(NYC_DIMENSION, NYC_SIZE);
    md_trie<NYC_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    MdTrieBench<NYC_DIMENSION> bench(&mdtrie);
    p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);

    bench.insert(NYC_DATA_ADDR, "nyc_insert_micro" + identification_string, total_points_count, parse_line_nyc);
    bench.lookup("nyc_lookup_micro" + identification_string);
    bench.range_search(NYC_QUERY_ADDR, "nyc_query_micro" + identification_string, get_query_nyc);
}

void tpch_bench(void) {

    optimization_code = OPTIMIZATION_SM;
    use_tpch_setting(TPCH_DIMENSION, TPCH_SIZE);
    md_trie<TPCH_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    MdTrieBench<TPCH_DIMENSION> bench(&mdtrie);
    p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);

    bench.insert(TPCH_DATA_ADDR, "tpch_insert_micro" + identification_string, total_points_count, parse_line_tpch);
    bench.lookup("tpch_lookup_micro" + identification_string);
    bench.range_search(TPCH_QUERY_ADDR, "tpch_query_micro" + identification_string, get_query_tpch);
}

const int DIMENSIONS = 9;
void sensitivity_num_dimensions(void) {

    optimization_code = OPTIMIZATION_SM;
    use_tpch_setting(DIMENSIONS, TPCH_SIZE);
    md_trie<DIMENSIONS> mdtrie(max_depth, trie_depth, max_tree_node);
    MdTrieBench<DIMENSIONS> bench(&mdtrie);

    bench.insert(TPCH_DATA_ADDR, "tpch_insert_dimensions_sensitivity" + identification_string, total_points_count, parse_line_tpch);
    bench.range_search_random("tpch_query_dimensions_sensitivity" + identification_string, get_random_query_tpch, 2000, 1000);
}

void sensitivity_selectivity(void) {

    optimization_code = OPTIMIZATION_SM;
    use_tpch_setting(TPCH_DIMENSION, TPCH_SIZE);
    md_trie<TPCH_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    MdTrieBench<TPCH_DIMENSION> bench(&mdtrie);

    bench.insert(TPCH_DATA_ADDR, "tpch_insert_selectivity_sensitivity" + identification_string, total_points_count, parse_line_tpch);
    bench.range_search_random("tpch_query_selectivity_sensitivity" + identification_string, get_random_query_tpch, total_points_count, 1);
}

int main(int argc, char *argv[]) {

    std::string argvalue;
    int arg;

    while ((arg = getopt (argc, argv, "b:")) != -1)
        switch (arg) {
            case 'b':
                argvalue = std::string(optarg);
                break;
            default:
                abort ();
        }
    
    std::cout << "benchmark: " << argvalue << std::endl;
    if (argvalue == "tpch") 
        tpch_bench();
    else if (argvalue == "github")
        github_bench();
    else if (argvalue == "nyc")
        nyc_bench();
    else if (argvalue == "sensitivity_num_dimensions")
        sensitivity_num_dimensions();
    else if (argvalue == "sensitivity_selectivity") 
        sensitivity_selectivity();
    else 
        std::cout << "Unrecognized benchmark: " << argvalue << std::endl;
}