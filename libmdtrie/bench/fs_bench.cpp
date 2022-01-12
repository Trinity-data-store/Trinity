#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <iostream>
#include <fstream>

const dimension_t DIMENSION = 7;

void run_bench(level_t max_depth, level_t trie_depth, preorder_t max_tree_node){

    std::vector<int32_t> found_points;
    md_trie<DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    data_point<DIMENSION> leaf_point;

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../data/fs/fs_dataset.txt", "r");

    // If the file cannot be open
    if (fp == nullptr)
    {
        fprintf(stderr, "file not found\n");
        exit(EXIT_FAILURE);
    }
    
    n_leaves_t n_points = 0;
    uint64_t max[DIMENSION];
    uint64_t min[DIMENSION];

    TimeStamp start, diff;
    diff = 0;

    /**
     * Insertion
     */

    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *token = strtok(line, " ");
        char *ptr;

        for (uint8_t i = 1; i <= 2; i ++){
            token = strtok(nullptr, " ");
        }

        for (dimension_t i = 0; i < DIMENSION; i++){
            token = strtok(nullptr, " ");
            leaf_point.set_coordinate(i, strtoul(token, &ptr, 10));
        }

        for (dimension_t i = 0; i < DIMENSION; i++){
            
            if (n_points == 0){
                max[i] = leaf_point.get_coordinate(i);
                min[i] = leaf_point.get_coordinate(i);
            }
            else {
                if (leaf_point.get_coordinate(i) > max[i]){
                    max[i] = leaf_point.get_coordinate(i);
                }
                if (leaf_point.get_coordinate(i) < min[i]){
                    min[i] = leaf_point.get_coordinate(i);
                }
            }       
        }
        start = GetTimestamp();
        mdtrie.insert_trie(&leaf_point, n_points);
        diff += GetTimestamp() - start;
        n_points ++;

        if (n_points % (total_points_count / 20) == 0)
            std::cout << "Inserted - n_points: " << n_points << std::endl;

        if (n_points == total_points_count)
            break;
    }

    std::cout << "Insertion Latency: " << (float) diff / total_points_count << std::endl;
    std::cout << "mdtrie storage: " << mdtrie.size() << std::endl;
    
    /**
     * Benchmark range search given a query selectivity (1000-2000)
     */

    data_point<DIMENSION> start_range;
    data_point<DIMENSION> end_range;

    line = nullptr;
    len = 0;
    fp = fopen("../queries/fs/fs_range_queries.csv", "r");
    read = getline(&line, &len, fp);
    diff = 0;
    int count = 0;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *ptr;
        char *token = strtok(line, ",");

        for (dimension_t i = 0; i < DIMENSION - 1; i++){
            token = strtok(nullptr, ","); // start
            start_range.set_coordinate(i, strtoul(token, &ptr, 10));
            token = strtok(nullptr, ","); // end
            end_range.set_coordinate(i, strtoul(token, &ptr, 10));
        }
        if (DIMENSION > 6){
            start_range.set_coordinate(6, min[6]);
            end_range.set_coordinate(6, max[6]);
        }

        std::vector<int32_t> found_points_temp;
        start = GetTimestamp();
        mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points_temp);
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;
        count ++;
        found_points_temp.clear();
    }
    std::cout << "Average query latency: " << (float) diff / count << std::endl;    

    /**
     * Point lookup given primary keys returned by range search
     */

    TimeStamp diff_primary = 0;
    n_leaves_t checked_points_size = 0;

    for (n_leaves_t primary_key = 0; primary_key < total_points_count; primary_key += 100){

        std::vector<morton_t> node_path_from_primary(max_depth + 1);

        tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *) (p_key_to_treeblock_compact->At(primary_key));
        
        start = GetTimestamp();
        morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(primary_key, node_path_from_primary);
        node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;
        data_point<DIMENSION> *coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);
        delete coordinates;
        diff_primary += GetTimestamp() - start;
        checked_points_size++;

    }
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

    level_t treeblock_size = 512;
    uint32_t trie_depth = 10;
    level_t max_depth = 32;
    discount_factor = 1;
    total_points_count = 14583357 / discount_factor;

    std::cout << "dimension: " << DIMENSION << std::endl;
    std::cout << "trie depth: " << trie_depth << std::endl;
    std::cout << "treeblock sizes: " << treeblock_size << std::endl;
    std::cout << "discount factor: " << discount_factor << std::endl;
    std::cout << "total_points_count: " << total_points_count << std::endl;

    bitmap::CompactPtrVector tmp_ptr_vect(total_points_count);
    p_key_to_treeblock_compact = &tmp_ptr_vect;

    std::vector<level_t> bit_widths = {32, 32, 32, 32, 24, 24, 32}; // 7 Dimensions    
    std::vector<level_t> start_bits = {0, 0, 0, 0, 0, 0, 0}; // 7 Dimensions    

    create_level_to_num_children(bit_widths, start_bits, max_depth);

    if (DIMENSION != bit_widths.size() || DIMENSION != start_bits.size()){
        std::cerr << "DATA DIMENSION does not match bit_widths vector!" << std::endl;
        exit(0);
    }

    if (total_points_count != p_key_to_treeblock_compact->get_num_elements()){
        std::cerr << "total_points_count does not match" << std::endl;
        exit(0);
    }
    run_bench(max_depth, trie_depth, treeblock_size);
}