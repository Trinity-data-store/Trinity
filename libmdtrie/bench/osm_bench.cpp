#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <tqdm.h>
#include <vector>
#include <iostream>
#include <fstream>

const dimension_t DIMENSION = 4;

void run_bench(level_t max_depth, level_t trie_depth, preorder_t max_tree_node){
    
    auto *found_points = new point_array<DIMENSION>();
    auto *all_points = new std::vector<data_point<DIMENSION>>;

    auto *mdtrie = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point<DIMENSION>();

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../data/osm/osm.csv", "r");

    // If the file cannot be open
    if (fp == nullptr)
    {
        fprintf(stderr, "file not found\n");
        exit(EXIT_FAILURE);
    }
    
    n_leaves_t n_points = 0;
    uint64_t max[DIMENSION];
    uint64_t min[DIMENSION];

    tqdm bar;
    TimeStamp start, diff;

    diff = 0;
    read = getline(&line, &len, fp);

    /**
     * Insertion
     */

    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar.progress(n_points, total_points_count);
        char *token = strtok(line, ","); 
        char *ptr;
      
        for (dimension_t i = 0; i < DIMENSION; i++){

            token = strtok(nullptr, ",");
            leaf_point->set_coordinate(i, strtoul(token, &ptr, 10));
        }

        for (dimension_t i = 0; i < DIMENSION; i++){
            if (n_points == 0){
                max[i] = leaf_point->get_coordinate(i);
                min[i] = leaf_point->get_coordinate(i);
            }
            else {
                if (leaf_point->get_coordinate(i) > max[i]){
                    max[i] = leaf_point->get_coordinate(i);
                }
                if (leaf_point->get_coordinate(i) < min[i]){
                    min[i] = leaf_point->get_coordinate(i);
                }
            }            
        }

        (*all_points).push_back((*leaf_point));

        start = GetTimestamp();
        mdtrie->insert_trie(leaf_point, n_points);
        diff += GetTimestamp() - start;

        n_points ++;
    }
    bar.finish();

    std::cout << "Insertion Latency: " << (float) diff / total_points_count << std::endl;
    std::cout << "mdtrie storage: " << mdtrie->size() << std::endl;

    /**
     * check whether a data point exists given coordinates
     */
    
    tqdm bar2;
    TimeStamp check_diff = 0;
    
    for (uint64_t i = 0; i < total_points_count; i++){
        bar2.progress(i, total_points_count);
        auto check_point = (*all_points)[i];
        start = GetTimestamp();
        if (!mdtrie->check(&check_point)){
            raise(SIGINT);
        } 
        check_diff += GetTimestamp() - start;  
    }
    bar2.finish();
    std::cout << "Average time to check one point: " << (float) check_diff / total_points_count << std::endl;

    /**
     * Benchmark range search given a query selectivity
     */

    auto *start_range = new data_point<DIMENSION>();
    auto *end_range = new data_point<DIMENSION>();

    int itr = 0;
    std::ofstream file("range_search_osm.csv", std::ios_base::app);
    srand(time(NULL));

    tqdm bar3;
    while (itr < 300){
        bar3.progress(itr, 300);

        for (uint8_t j = 0; j < DIMENSION; j++){
            start_range->set_coordinate(j, min[j] + (max[j] - min[j] + 1) / 10 * (rand() % 10));
            end_range->set_coordinate(j, start_range->get_coordinate(j) + (max[j] - start_range->get_coordinate(j) + 1) / 3 * (rand() % 3));
        }
        auto *found_points_temp = new point_array<DIMENSION>();
        start = GetTimestamp();
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points_temp);
        diff = GetTimestamp() - start;
        if (primary_key_vector.size() >= 1000){
            file << primary_key_vector.size() << "," << diff << "," << std::endl;
            itr ++;
        }

        primary_key_vector.clear();
    }
    bar3.finish();

    /**
     * Range Search with full range
     */
    
    for (dimension_t i = 0; i < DIMENSION; i++){
        start_range->set_coordinate(i, min[i]);
        end_range->set_coordinate(i, max[i]);
    }

    start = GetTimestamp();
    mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
    diff = GetTimestamp() - start;

    std::cout << "found_pts size: " << found_points->size() << std::endl;
    std::cout << "Range Search Latency: " << (float) diff / found_points->size() << std::endl;

    /**
     * Point lookup given primary keys returned by range search
     */

    n_leaves_t found_points_size = found_points->size();
    TimeStamp diff_primary = 0;

    n_leaves_t checked_points_size = 0;
    tqdm bar4;
    for (n_leaves_t i = 0; i < found_points_size; i += 5){
        checked_points_size++;

        bar4.progress(i, found_points_size);
        data_point<DIMENSION> *point = found_points->at(i);
        n_leaves_t returned_primary_key = point->read_primary();
        morton_t *node_path_from_primary = (morton_t *)malloc((max_depth + 1) * sizeof(morton_t));

        tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *) (p_key_to_treeblock_compact.At(returned_primary_key));
        
        start = GetTimestamp();
        morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(returned_primary_key, node_path_from_primary);
        node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;
        data_point<DIMENSION> *returned_coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);
        diff_primary += GetTimestamp() - start;

        for (dimension_t j = 0; j < DIMENSION; j++){
            if (returned_coordinates->get_coordinate(j) != point->get_coordinate(j)){
                raise(SIGINT);
            }
        }    
        auto correct_point = (* all_points)[returned_primary_key];
        for (dimension_t j = 0; j < DIMENSION; j++){
            if (returned_coordinates->get_coordinate(j) != correct_point.get_coordinate(j)){
                raise(SIGINT);
            }
        }          
        free(node_path_from_primary);
    }
    bar4.finish();
    std::cout << "Lookup Latency: " << (float) diff_primary / checked_points_size << std::endl;
}

int main() {

    /**
     * Set hyperparameters
     * treeblock_size: maximum number of nodes a treeblock can hode
     * trie_depth: the maximum level of the top-level pointer-based trie structure
     * max_depth: the depth of whole data structure
     * bit_widths: the bit widths of each column, with default start-level all set to 0.
     */

    level_t trie_depth = 6;
    uint32_t treeblock_size = 512;
    total_points_count = 152806264;
    // p_key_to_treeblock_compact(bitmap::CompactPtrVector(total_points_count));

    std::vector<level_t> bit_widths = {32, 32, 32, 32}; // 4 Dimensions
    std::vector<level_t> start_bits = {0, 0, 0, 0}; // 4 Dimensions    

    std::cout << "dimension: " << DIMENSION << std::endl;
    std::cout << "trie depth: " << trie_depth << std::endl;
    std::cout << "treeblock sizes: " << treeblock_size << std::endl;
    std::cout << "discount factor: " << discount_factor << std::endl;

    level_t max_depth = 32;
    create_level_to_num_children(bit_widths, start_bits, max_depth);

    if (DIMENSION != bit_widths.size() || DIMENSION != start_bits.size()){
        std::cerr << "DATA DIMENSION does not match bit_widths vector!" << std::endl;
        exit(0);
    }

    if (total_points_count != 152806264){
        std::cerr << "total_points_count does not match" << std::endl;
        exit(0);
    }

    run_bench(max_depth, trie_depth, treeblock_size);
    std::cout << std::endl;
}