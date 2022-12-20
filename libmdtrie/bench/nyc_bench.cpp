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

    use_nyc_setting();
    md_trie<NYC_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    TrinityBench<NYC_DIMENSION> bench(&mdtrie);

    bench.insert(NYC_DATA_ADDR, "nyc_insert_micro" + identification_string, total_points_count, parse_line_nyc);
    bench.lookup("nyc_lookup_micro" + identification_string);
    bench.range_search(NYC_QUERY_ADDR, "nyc_query_micro" + identification_string, get_query_nyc);
}