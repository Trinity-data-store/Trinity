#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <iostream>
#include <fstream>
#include "../../librpc/src/TrinityParseFIle.h"

#define GITHUB_SIZE 200000000 // 200M
// #define GENERATE_QUERY 
const dimension_t DIMENSION = 10;
level_t max_depth = 32;
level_t trie_depth = 6;
preorder_t max_tree_node = 512;
point_t points_to_insert = 30000;
point_t points_for_warmup = points_to_insert / 5;
point_t skip_size = 0;
std::string identification_string;

// #define NO_MORTON_CODE_OPT
// #define GENERALIZE_MORTON_CODE_EXP
#define STAGGER_MORTON_CODE_EXP

void flush_vector_to_file(std::vector<TimeStamp> vect, std::string filename){
    std::ofstream outFile(filename);
    for (const auto &e : vect) outFile << std::to_string(e) << "\n";
}

#ifdef GENERATE_QUERY
int gen_rand(int start, int end) {
    return start + ( std::rand() % ( end - start + 1 ) );
}
#endif

void run_bench(){

    std::vector<std::vector<int32_t>> found_points;
    md_trie<DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    data_point<DIMENSION> leaf_point;

    std::ifstream infile("/mntData/github_events_processed_9.csv");
    std::string line;
    point_t n_points = 0;
    std::vector<TimeStamp> insertion_latency_vect;
    std::vector<TimeStamp> lookup_latency_vect;
    TimeStamp start, diff = 0;

    /**
     * Insertion
     */
    point_t n_skipped = 0;

    while (std::getline(infile, line))
    {
        if (n_skipped < skip_size) {
            n_skipped ++;
            continue;
        }
        std::vector<int32_t> vect = parse_line_github(line);
        data_point<DIMENSION> leaf_point;

        for (dimension_t i = 0; i < DIMENSION; i++) {
            leaf_point.set_coordinate(i, vect[i]);
        }

        start = GetTimestamp();
        mdtrie.insert_trie(&leaf_point, n_points, p_key_to_treeblock_compact);
        TimeStamp latency =  GetTimestamp() - start;
        diff += latency;
        n_points ++;

        if (n_points == total_points_count)
            break;

        if (n_points > total_points_count - points_to_insert)
            insertion_latency_vect.push_back(latency);

        if (n_points % (total_points_count / 50) == 0)
            std::cout << "n_points: "  << n_points << std::endl;

        num_treeblock_expand = 0;
    }    

    std::cout << "Insertion Latency: " << (float) diff / n_points << std::endl;
    std::cout << "mdtrie storage: " << mdtrie.size(p_key_to_treeblock_compact) << std::endl;
    std::ofstream out_storage("/proj/trinity-PG0/Trinity/results/sensitivity_optimization/github_storage_" + identification_string);
    out_storage << "mdtrie storage: " << mdtrie.size(p_key_to_treeblock_compact) << std::endl;
    // flush_vector_to_file(insertion_latency_vect, "/proj/trinity-PG0/Trinity/results/sensitivity_optimization/github_insert_" + identification_string);

    infile.close();
 
    /**
     * Point Lookup
     */

    TimeStamp cumulative = 0;

    for (point_t i = 0; i < points_to_insert; i ++) {

        std::vector<morton_t> node_path_from_primary(max_depth + 1);

        tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *) (p_key_to_treeblock_compact->At(i));

        start = GetTimestamp();
        morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(i, node_path_from_primary);
        node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;
        data_point<DIMENSION> *coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);

        TimeStamp temp_diff = GetTimestamp() - start;
        cumulative += temp_diff;
        if (!coordinates) {
            std::cerr << "range search failed!" << std::endl;
            exit(-1);
        }

        if (i <= points_to_insert)
            lookup_latency_vect.push_back(temp_diff);

    }
    std::cout << "Done! " << "Lookup Latency per point: " << (float) cumulative / points_to_insert << std::endl;
    // flush_vector_to_file(lookup_latency_vect, "/proj/trinity-PG0/Trinity/results/sensitivity_optimization/github_lookup_" + identification_string);

    /** 
        Range Search
    */

    char *infile_address = (char *)"/proj/trinity-PG0/Trinity/results/sensitivity_optimization/github_search_base_10";
    std::string outfile_address = "/proj/trinity-PG0/Trinity/results/sensitivity_optimization/github_search_" + identification_string + "_random_query";
    std::string search_outfile_address = "/proj/trinity-PG0/Trinity/results/sensitivity_optimization/github_search_base_" + std::to_string(DIMENSION);
    std::ofstream search_outfile(search_outfile_address, std::ios_base::app);

    //   [events_count, authors_count, forks, stars, issues, pushes, pulls, downloads, start_date, end_date]

    std::vector<int32_t> max_values = {7451541, 737170, 262926, 354850, 379379, 3097263, 703341, 8745, 20201206, 20201206};
    std::vector<int32_t> min_values = {1, 1, 0, 0, 0, 0, 0, 0, 20110211, 20110211};

    std::ifstream file(infile_address);
    std::ofstream outfile(outfile_address);

    int i = 0;
    while (i < 1000) {

        std::vector<int32_t> found_points;
        data_point<DIMENSION> start_range;
        data_point<DIMENSION> end_range;

        #ifdef GENERATE_QUERY
        for (dimension_t j = 0; j < DIMENSION; j++){
            if (j <= 7)
                start_range.set_coordinate(j, gen_rand(min_values[j], 10));
            else
                start_range.set_coordinate(j, gen_rand(min_values[j], max_values[j]));
            end_range.set_coordinate(j, gen_rand(start_range.get_coordinate(j), max_values[j]));
        }

        #else

        std::string line;
        std::getline(file, line);

        std::stringstream ss(line);
        for (dimension_t j = 0; j < DIMENSION; j++){
            std::string str;
            std::getline(ss, str, ',');
            start_range.set_coordinate(j, static_cast<int32_t>(std::stoul(str)));
        }
        for (dimension_t j = 0; j < DIMENSION; j++){
            std::string str;
            std::getline(ss, str, ',');
            end_range.set_coordinate(j, static_cast<int32_t>(std::stoul(str)));
        }
        #endif

        start = GetTimestamp();
        mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points);
        diff = GetTimestamp() - start;

        std::cout << "i: " << i << ", " << found_points.size() << std::endl;
        for (dimension_t j = 0; j < DIMENSION; j++) {
            std::cout << start_range.get_coordinate(j) << ",";
        }
        for (dimension_t j = 0; j < DIMENSION; j++) {
            std::cout << end_range.get_coordinate(j) << ",";
        }
        std::cout << std::endl;
        
        #ifdef GENERATE_QUERY
        if (found_points.size() / DIMENSION < 1000 || found_points.size() / DIMENSION > 2000)
            continue;
        #endif

        #ifdef GENERATE_QUERY
        for (dimension_t j = 0; j < DIMENSION; j++) {
            search_outfile << start_range.get_coordinate(j) << ",";
        }
        for (dimension_t j = 0; j < DIMENSION; j++) {
            search_outfile << end_range.get_coordinate(j) << ",";
        }
        search_outfile << std::endl;
        #else
        outfile << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / DIMENSION << std::endl;
        #endif
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

    std::vector<level_t> bit_widths = {24, 24, 24, 24, 24, 24, 24, 16, 24, 24}; // 10 Dimensions;
    std::vector<level_t> start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 10 Dimensions;

    #ifdef NO_MORTON_CODE_OPT
    bit_widths = {24, 24, 24, 24, 24, 24, 24, 24, 24, 24};
    start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    identification_string = "no_morton_code_opt";


    #endif

    #ifdef GENERALIZE_MORTON_CODE_EXP
    bit_widths = {24, 24, 24, 24, 24, 24, 24, 16, 24, 24}; 
    start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    identification_string = "generalize_morton_code_exp";


    #endif

    // [events_count, authors_count, forks, stars, issues, pushes, pulls, downloads, start_date, end_date]
    // [start_date, end_date, stars, forks, events_count, issues]

    #ifdef STAGGER_MORTON_CODE_EXP
    bit_widths = {24, 24, 24, 24, 24, 24, 24, 16 + 8, 24, 24}; 
    start_bits = {0, 0, 0, 0, 0, 0, 0, 8, 0, 0};
    identification_string = "stagger_morton_code_exp";
    #endif 

    #ifdef COLLAPSED_NODE_EXP
    bit_widths = {24, 24, 24, 24, 24, 24, 24, 24, 24, 24};
    start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    identification_string = "collapsed_node_exp";
    #endif

    trie_depth = 6;
    max_depth = 24;
    no_dynamic_sizing = true;
    total_points_count = GITHUB_SIZE; 
    skip_size = 700000000 - total_points_count;
    max_tree_node = 512;

    #ifdef COLLAPSED_NODE_EXP_REDUCED
    identification_string = "collapsed_node_exp_reduced";
    total_points_count /= 50;
    #endif

    // total_points_count /= 10;
    std::cout << "identification_string: " << identification_string << std::endl;

    bitmap::CompactPtrVector tmp_ptr_vect(total_points_count);
    p_key_to_treeblock_compact = &tmp_ptr_vect;

    create_level_to_num_children(bit_widths, start_bits, max_depth);
    run_bench();
}