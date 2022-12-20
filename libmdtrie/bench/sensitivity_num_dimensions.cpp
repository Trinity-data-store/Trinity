#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <iostream>
#include <fstream>
#include "parser.hpp"
#include <random>
#include "common.hpp"
#include "benchmark.hpp"

const int DIMENSIONS = 9;

int main() {

    use_tpch_setting(DIMENSIONS);
    md_trie<DIMENSIONS> mdtrie(max_depth, trie_depth, max_tree_node);
    MdTrieBench<DIMENSIONS> bench(&mdtrie);

    bench.insert(TPCH_DATA_ADDR, "tpch_insert_dimensions_sensitivity" + identification_string, total_points_count, parse_line_tpch);
    bench.range_search_random("tpch_query_dimensions_sensitivity" + identification_string, get_random_query_tpch, 2000, 1000);
}