#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <iostream>
#include <fstream>
#include <string> 
#include "../../librpc/src/TrinityParseFIle.h"

#define LATENCY_BENCH
#define NEW_LATENCY_BENCH
#define NYC_SIZE 200000000 // 200M

const dimension_t DIMENSION = 15;
level_t max_depth = 32;
level_t trie_depth = 6;
preorder_t max_tree_node = 512;
point_t points_to_insert = 30000;
point_t points_for_warmup = points_to_insert / 5;

void flush_vector_to_file(std::vector<TimeStamp> vect, std::string filename){
    #ifdef NEW_LATENCY_BENCH
    std::ofstream outFile(filename + "_new");
    #else
    std::ofstream outFile(filename);
    #endif
    for (const auto &e : vect) outFile << std::to_string(e) << "\n";
}

void run_bench(){

    std::vector<std::vector<int32_t>> found_points;
    md_trie<DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    data_point<DIMENSION> leaf_point;

    std::ifstream infile("/mntData2/nyc_taxi/nyc_taxi_processed_675.csv");
    std::string line;
    point_t n_points = 0;
    std::vector<TimeStamp> insertion_latency_vect;
    std::vector<TimeStamp> lookup_latency_vect;
    TimeStamp start, diff = 0;

    /**
     * Insertion
     */

    while (std::getline(infile, line))
    {
        std::vector<int32_t> vect = parse_line_nyc(line);
        data_point<DIMENSION> leaf_point;

        for (dimension_t i = 0; i < DIMENSION; i++) {
            leaf_point.set_coordinate(i, vect[i]);
        }

        start = GetTimestamp();
        mdtrie.insert_trie(&leaf_point, n_points, p_key_to_treeblock_compact);
        TimeStamp latency =  GetTimestamp() - start;
        diff += latency;
        n_points ++;

        #if defined(NEW_LATENCY_BENCH)
        if (n_points == NYC_SIZE)
            break;
        #elif defined(LATENCY_BENCH)
        if (n_points == points_to_insert)
            break;    
        #else
        if (n_points == NYC_SIZE)
            break;
        #endif

        #ifdef NEW_LATENCY_BENCH
        if (n_points > NYC_SIZE - points_to_insert + points_for_warmup)
            insertion_latency_vect.push_back(latency);
        #else
        if (n_points > points_for_warmup && n_points <= points_to_insert)
            insertion_latency_vect.push_back(latency);
        #endif

        if (n_points % (total_points_count / 50) == 0)
            std::cout << "n_points: "  << n_points << std::endl;
    }    

    std::cout << "Insertion Latency: " << (float) diff / n_points << std::endl;
    std::cout << "mdtrie storage: " << mdtrie.size(p_key_to_treeblock_compact) << std::endl;
    infile.close();
    flush_vector_to_file(insertion_latency_vect, "/proj/trinity-PG0/Trinity/results/latency_cdf/trinity_nyc_insert_micro");

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

        if (i > points_for_warmup && i <= points_to_insert)
            lookup_latency_vect.push_back(temp_diff);
    }
    std::cout << "Done! " << "Lookup Latency per point: " << (float) cumulative / points_to_insert << std::endl;
    flush_vector_to_file(lookup_latency_vect, "/proj/trinity-PG0/Trinity/results/latency_cdf/trinity_nyc_lookup_micro");

    /** 
        Range Search
    */

    #ifdef LATENCY_BENCH
    return;
    #endif
    
    char *infile_address = (char *)"/proj/trinity-PG0/Trinity/queries/nyc/nyc_query_new_converted";
    char *outfile_address = (char *)"/proj/trinity-PG0/Trinity/results/nyc_trinity_micro";

    std::vector<int32_t> max_values = {20160630, 20221220, 899, 898, 899, 898, 255, 198623000, 21474808, 1000, 1312,  3950589, 21474836, 138, 21474830};
    std::vector<int32_t> min_values = {20090101, 19700101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    std::ifstream file(infile_address);
    std::ofstream outfile(outfile_address);

    for (int i = 0; i < 1000; i ++) {

        std::vector<int32_t> found_points;
        data_point<DIMENSION> start_range;
        data_point<DIMENSION> end_range;

        for (dimension_t i = 0; i < DIMENSION; i++){
            start_range.set_coordinate(i, min_values[i]);
            end_range.set_coordinate(i, max_values[i]);
        }

        std::string line;
        std::getline(file, line);

        std::stringstream ss(line);

        while (ss.good()) {

            std::string index_str;
            std::getline(ss, index_str, ',');

            std::string start_range_str;
            std::getline(ss, start_range_str, ',');
            std::string end_range_str;
            std::getline(ss, end_range_str, ',');

            int index = std::stoul(index_str);
            if (start_range_str != "-1") {
                if (index >= 2 && index <= 5) {
                    float num_float = std::stof(start_range_str);
                    start_range.set_coordinate(index, static_cast<int32_t>(num_float * 10));                    
                }
                else 
                    start_range.set_coordinate(index, std::stoul(start_range_str));
            }
            if (end_range_str != "-1") {
                if (index >= 2 && index <= 5) {
                    float num_float = std::stof(end_range_str);
                    end_range.set_coordinate(index, static_cast<int32_t>(num_float * 10));                    
                }
                else
                    end_range.set_coordinate(index, std::stoul(end_range_str));
            }
        }

        start_range.set_coordinate(0, start_range.get_coordinate(0) - 20090000);
        start_range.set_coordinate(1, start_range.get_coordinate(1) - 19700000);
        end_range.set_coordinate(0, end_range.get_coordinate(0) - 20090000);
        end_range.set_coordinate(1, end_range.get_coordinate(1) - 19700000);

        start = GetTimestamp();
        mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points);
        diff = GetTimestamp() - start;
        outfile << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / DIMENSION << std::endl;
        found_points.clear();
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

    std::vector<level_t> bit_widths = {18, 20, 10, 10, 10 + 18, 10 + 18, 8, 28, 25, 10 + 18, 11 + 17, 22, 25, 8 + 20, 25}; 
    std::vector<level_t> start_bits = {0, 0, 0, 0, 0 + 18, 0 + 18, 0, 0, 0, 0 + 18, 0 + 17, 0, 0, 0 + 20, 0}; 

    trie_depth = 6;
    max_depth = 28;
    no_dynamic_sizing = true;
    total_points_count = 200000000; 

    bitmap::CompactPtrVector tmp_ptr_vect(total_points_count);
    p_key_to_treeblock_compact = &tmp_ptr_vect;

    create_level_to_num_children(bit_widths, start_bits, max_depth);
    run_bench();
}