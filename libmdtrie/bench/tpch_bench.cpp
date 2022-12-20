#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <fstream>
#include "../../librpc/src/TrinityParseFIle.h"
#include "common.h"

void run_bench(){

    std::vector<int32_t> found_points;
    md_trie<TPCH_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    data_point<TPCH_DIMENSION> leaf_point;

    std::ifstream infile(TPCH_DATA_ADDR);
    point_t n_points = 0;
    std::vector<TimeStamp> insertion_latency_vect;
    std::vector<TimeStamp> lookup_latency_vect;
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

        if (n_points > TPCH_SIZE - points_to_insert)
            insertion_latency_vect.push_back(latency);

        if (n_points == TPCH_SIZE)
            break; 
    }    

    std::cout << "Insertion Latency: " << (float) diff / n_points << std::endl;
    std::cout << "mdtrie storage: " << mdtrie.size(p_key_to_treeblock_compact) << std::endl;
    flush_vector_to_file(insertion_latency_vect, results_folder_addr + "trinity_tpch_insert_micro");
    infile.close();

    /**
     * Point Lookup
     */

    TimeStamp cumulative = 0;

    for (point_t i = 0; i < points_to_lookup; i ++) {

        start = GetTimestamp();
        std::vector<morton_t> node_path_from_primary(max_depth + 1);
        tree_block<TPCH_DIMENSION> *t_ptr = (tree_block<TPCH_DIMENSION> *) (p_key_to_treeblock_compact->At(i));
        morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(i, node_path_from_primary);
        node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;
        t_ptr->node_path_to_coordinates(node_path_from_primary, TPCH_DIMENSION);
        TimeStamp temp_diff = GetTimestamp() - start;
        cumulative += temp_diff;

        lookup_latency_vect.push_back(temp_diff);
    }
    flush_vector_to_file(lookup_latency_vect, results_folder_addr + "trinity_tpch_lookup_micro");
    std::cout << "Done! " << "Lookup Latency per point: " << (float) cumulative / points_to_lookup << std::endl;

    /** 
        Range Search
    */

    std::string outfile_address = results_folder_addr + "tpch_trinity_micro" + identification_string;
    std::ifstream file(TPCH_QUERY_ADDR);
    std::ofstream outfile(outfile_address);

    for (int i = 0; i < QUERY_NUM; i ++) {

        std::vector<int32_t> found_points;
        data_point<TPCH_DIMENSION> start_range;
        data_point<TPCH_DIMENSION> end_range;

        std::string line;
        std::getline(file, line);
        get_query_tpch(line, &start_range, &end_range);

        start = GetTimestamp();
        mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points);
        diff = GetTimestamp() - start;
        outfile << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / TPCH_DIMENSION << std::endl;
        found_points.clear();
    }
}

int main() {

    use_default_tpch_setting(TPCH_SIZE);
    run_bench();
}