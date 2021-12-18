#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <tqdm.h>
#include <vector>
#include <iostream>
#include <fstream>

// const int DATA_DIMENSION = 8;
level_t TRIE_DEPTH = 10;
uint32_t TREEBLOCK_SIZE = 1024;
std::ofstream myfile;

void run_bench(level_t max_depth, level_t trie_depth, preorder_t max_tree_node, std::vector<level_t> dimension_bits){
    
    create_level_to_num_children(dimension_bits, max_depth);
    auto *found_points = new point_array();
    auto *all_points = new std::vector<data_point>;

    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point();

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("/home/ziming/osm/osm_us_northeast_timestamp.csv", "r");

    // If the file cannot be open
    if (fp == nullptr)
    {
        fprintf(stderr, "file not found\n");
        exit(EXIT_FAILURE);
    }
    
    n_leaves_t n_points = 0;
    uint64_t max[DATA_DIMENSION];
    uint64_t min[DATA_DIMENSION];
    n_leaves_t n_lines = 152806264 / 10;
    total_points_count = n_lines;

    tqdm bar;
    TimeStamp start, diff;

    diff = 0;
    read = getline(&line, &len, fp);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar.progress(n_points, n_lines);
        char *token = strtok(line, ","); // id
        char *ptr;
      
        for (dimension_t i = 0; i < DATA_DIMENSION; i++){

            token = strtok(nullptr, ",");
            leaf_point->set_coordinate(i, strtoul(token, &ptr, 10));
        }

        for (dimension_t i = 0; i < DATA_DIMENSION; i++){
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
        if (n_points == n_lines)
            break;
    }
    bar.finish();

    myfile << "Insertion Latency: " << (float) diff / n_lines << std::endl;
    myfile << "mdtrie storage: " << mdtrie->size() << std::endl;
    std::cout << "mdtrie storage: " << mdtrie->size() << std::endl;
    exit(0);
    myfile << "trie_size: " << trie_size << std::endl;
    myfile << "p_key_to_treeblock_compact_size: " << p_key_to_treeblock_compact_size << std::endl;
    myfile << "total_treeblock_num: " << total_treeblock_num << std::endl;
    myfile << "treeblock_primary_pointer_size: " << treeblock_primary_pointer_size << std::endl;
    myfile << "treeblock_primary_size: " << treeblock_primary_size << std::endl;
    myfile << "treeblock_nodes_size: " << treeblock_nodes_size << std::endl;
    myfile << "collapsed_node_num: " << collapsed_node_num << std::endl;

    /*
    tqdm bar2;
    TimeStamp check_diff = 0;
    
    for (uint64_t i = 0; i < n_lines; i++){
        bar2.progress(i, n_lines);
        auto check_point = (*all_points)[i];
        start = GetTimestamp();
        if (!mdtrie->check(&check_point)){
            raise(SIGINT);
        } 
        check_diff += GetTimestamp() - start;  
    }
    bar2.finish();
    myfile << "Average time to check one point: " << (float) check_diff / n_lines << std::endl;
    */

    line = nullptr;
    len = 0;
    fp = fopen("/home/ziming/phtree-cpp/build/osm_phtree_queries_1000.csv", "r");
    int count = 0;
    diff = 0;
    std::ofstream range_myfile("osm_mdtrie_range_queries.csv");

    while ((read = getline(&line, &len, fp)) != -1)
    {
        data_point start_range;
        data_point end_range;      
        char *ptr;

        char *token = strtok(line, ","); // id
        token = strtok(nullptr, ",");
        token = strtok(nullptr, ",");

        for (dimension_t i = 0; i < DATA_DIMENSION; i++){
            token = strtok(nullptr, ","); // id
            start_range.set_coordinate(i, strtoul(token, &ptr, 10));
            token = strtok(nullptr, ",");
            end_range.set_coordinate(i, strtoul(token, &ptr, 10));
        }

        int present_pt_count = 0;
        for (unsigned int i = 0; i < all_points->size(); i++){
            bool match = true;
            for (dimension_t j = 0; j < DATA_DIMENSION; j++){
                if ( (*all_points)[i].get_coordinate(j) < start_range.get_coordinate(j) || (*all_points)[i].get_coordinate(j) > end_range.get_coordinate(j)){
                    match = false;
                    break;
                }
            }
            if (match){
                present_pt_count ++;
            }
        }   
        std::cout << "present point count: " << present_pt_count << std::endl;

        point_array found_points_temp;
        start = GetTimestamp();
        mdtrie->range_search_trie(&start_range, &end_range, mdtrie->root(), 0, &found_points_temp);
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;

        count ++;   
        std::cout << "found_points_temp.size: " << primary_key_vector.size() << std::endl; 
        range_myfile << primary_key_vector.size() << "," << temp_diff << std::endl;
        std::cout << "diff: " << temp_diff << std::endl;
        primary_key_vector.clear();


    }
    std::cout << "average query latency: " << (float) diff / count << std::endl;
    
    exit(0);


    auto *start_range = new data_point();
    auto *end_range = new data_point();
/*
    int itr = 0;
    std::ofstream file("range_search_osm_only_primary_key.csv", std::ios_base::app);
    srand(time(NULL));

    tqdm bar3;
    while (itr < 300){
        bar3.progress(itr, 300);

        for (uint8_t j = 0; j < DATA_DIMENSION; j++){
            start_range->set_coordinate(j, min[j] + (max[j] - min[j] + 1) / 10 * (rand() % 10));
            end_range->set_coordinate(j, start_range->get_coordinate(j) + (max[j] - start_range->get_coordinate(j) + 1) / 3 * (rand() % 3));
        }

        auto *found_points_temp = new point_array();
        start = GetTimestamp();
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points_temp);
        diff = GetTimestamp() - start;

        // if (found_points_temp->size() >= 0.0005 * n_lines && found_points_temp->size() <= 0.0015 * n_lines){
        // if (found_points_temp->size() >= 1000){
        //     file << found_points_temp->size() << "," << diff << "," << std::endl;
        //     itr ++;
        // }
        if (primary_key_vector.size() >= 1000){
            file << primary_key_vector.size() << "," << diff << "," << std::endl;
            itr ++;
        }
        primary_key_vector.clear();
    }
    bar3.finish();
*/
    exit(0);
    
    for (dimension_t i = 0; i < DATA_DIMENSION; i++){
        start_range->set_coordinate(i, min[i]);
        end_range->set_coordinate(i, max[i]);
    }

    start = GetTimestamp();
    mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
    diff = GetTimestamp() - start;

    myfile << "found_pts size: " << found_points->size() << std::endl;
    myfile << "Range Search Latency: " << (float) diff / found_points->size() << std::endl;

    n_leaves_t found_points_size = found_points->size();
    TimeStamp diff_primary = 0;

    n_leaves_t checked_points_size = 0;
    tqdm bar4;
    for (n_leaves_t i = 0; i < found_points_size; i += 5){
        checked_points_size++;

        bar4.progress(i, found_points_size);
        data_point *point = found_points->at(i);
        n_leaves_t returned_primary_key = point->read_primary();

        symbol_t *node_path_from_primary = (symbol_t *)malloc((max_depth + 1) * sizeof(symbol_t));

        tree_block *t_ptr = (tree_block *) (p_key_to_treeblock_compact.At(returned_primary_key));
        
        start = GetTimestamp();

        symbol_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(returned_primary_key, node_path_from_primary);
        node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

        data_point *returned_coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary, DATA_DIMENSION);

        diff_primary += GetTimestamp() - start;

        for (dimension_t j = 0; j < DATA_DIMENSION; j++){
            if (returned_coordinates->get_coordinate(j) != point->get_coordinate(j)){
                raise(SIGINT);
            }
        }    

        auto correct_point = (* all_points)[returned_primary_key];
        for (dimension_t j = 0; j < DATA_DIMENSION; j++){
            if (returned_coordinates->get_coordinate(j) != correct_point.get_coordinate(j)){
                raise(SIGINT);
            }
        }          

        free(node_path_from_primary);
    }
    bar4.finish();
    myfile << "Lookup Latency: " << (float) diff_primary / checked_points_size << std::endl;
     
}

int main() {

    TREEBLOCK_SIZE = 512;
    TRIE_DEPTH = 6;
    myfile.open("osm_benchmark_northeast_" + std::to_string(DATA_DIMENSION) + "_" + std::to_string(TRIE_DEPTH) + "_" + std::to_string(TREEBLOCK_SIZE) + ".txt", std::ios_base::app);

    //  max: {183, 59, 23, 31, 12, 2021, 834785061, 498730330}
    //  min: {1, 0, 0, 1, 1, 2006, 95434670, 384409862}

    // std::vector<level_t> dimension_bits = {8, 32, 32, 32}; // 8 Dimensions
    // std::vector<level_t> new_start_dimension_bits = {0, 0, 0, 0}; // 8 Dimensions
    std::vector<level_t> dimension_bits = {32, 32, 32, 32}; // 8 Dimensions
    std::vector<level_t> new_start_dimension_bits = {0, 0, 0, 0}; // 8 Dimensions    

    start_dimension_bits = new_start_dimension_bits;   

    myfile << std::endl << "dimension_bits: ";
    for (uint8_t i = 0; i < dimension_bits.size(); i++){
        myfile << dimension_bits[i] << ", ";
    }
    myfile << std::endl;

    is_osm = true;

    myfile << "dimension: " << DATA_DIMENSION << std::endl;
    myfile << "trie depth: " << TRIE_DEPTH << std::endl;
    myfile << "treeblock sizes: " << TREEBLOCK_SIZE << std::endl;
    run_bench(32, TRIE_DEPTH, TREEBLOCK_SIZE, dimension_bits);
    myfile << std::endl;

}