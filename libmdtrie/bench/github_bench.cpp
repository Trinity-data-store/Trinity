#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <fstream>
#include "parser.hpp"
#include "common.hpp"
#include "benchmark.hpp"

int main() {
    
    use_github_setting();
    md_trie<GITHUB_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    MdTrieBench<GITHUB_DIMENSION> bench(&mdtrie);

    bench.insert(GITHUB_DATA_ADDR, "github_insert_micro" + identification_string, total_points_count, parse_line_github);
    bench.lookup("github_lookup_micro" + identification_string);
    bench.range_search(GITHUB_QUERY_ADDR, "github_query_micro" + identification_string, get_query_github);
}