#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <fstream>
#include "../../librpc/src/TrinityParseFile.h"
#include "common.hpp"
#include "benchmark.hpp"

int main() {

    use_tpch_setting(TPCH_DIMENSION);
    md_trie<TPCH_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    TrinityBench<TPCH_DIMENSION> bench(&mdtrie);

    bench.insert(TPCH_DATA_ADDR, "tpch_insert_micro" + identification_string, total_points_count, parse_line_tpch);
    bench.lookup("tpch_lookup_micro" + identification_string);
    bench.range_search(TPCH_QUERY_ADDR, "tpch_query_micro" + identification_string, get_query_tpch);
}