#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <iostream>
#include <fstream>
#include "../../librpc/src/TrinityParseFile.h"
#include <random>
#include "common.hpp"
#include "benchmark.hpp"

int main() {

    use_tpch_setting(TPCH_DIMENSION);
    md_trie<TPCH_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    TrinityBench<TPCH_DIMENSION> bench(&mdtrie);

    bench.insert(TPCH_DATA_ADDR, "tpch_insert_selectivity_sensitivity" + identification_string, total_points_count, parse_line_tpch);
    bench.range_search_random("tpch_query_selectivity_sensitivity" + identification_string, get_random_query_tpch, total_points_count, 1);
}