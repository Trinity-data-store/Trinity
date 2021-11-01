#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <tqdm.h>
#include <vector>
#include <iostream>
#include <fstream>
// Remove ID
const int DIMENSION = 6; // <= 8
level_t TRIE_DEPTH = 10;
const symbol_t NUM_BRANCHES = pow(2, DIMENSION);
uint32_t TREEBLOCK_SIZE = 1024;
std::ofstream myfile;
std::ofstream serialize_file;

void insert_for_node_path(point_array *found_points, level_t max_depth, level_t trie_depth, preorder_t max_tree_node, std::vector<data_point> *all_points){
    // to-do
    
    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point();

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../libmdtrie/bench/data/osm_combined_updated.csv", "r");

    // If the file cannot be open
    if (fp == nullptr)
    {
        fprintf(stderr, "file not found\n");
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            printf("Current working dir: %s\n", cwd);
        } else {
            perror("getcwd() error");
        }
        exit(EXIT_FAILURE);
    }

    
    n_leaves_t n_points = 0;
    uint64_t max[DIMENSION];
    uint64_t min[DIMENSION];
    n_leaves_t n_lines = 14252681;
    total_points_count = n_lines;

    tqdm bar;
    TimeStamp start, diff;
    diff = 0;

    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar.progress(n_points, n_lines);
        char *token = strtok(line, ",");
        char *ptr;
      
        for (dimension_t i = 0; i < 8; i++){

            if (i == 1){
                token = strtok(nullptr, ",");
                token = strtok(nullptr, ",");
                // Remove 2 all-zero column 
            }

            token = strtok(nullptr, ",");
   
            if (i < 8 - DIMENSION)
                continue;

            leaf_point->set_coordinate(i - (8 - DIMENSION), strtoul(token, &ptr, 10));
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
        // if (n_points == 1000000)
        //     break;
    }

    bar.finish();
    fprintf(stderr, "dimension: %d\n", DIMENSION);
    fprintf(stderr, "Average time to insert one point: %f microseconds per operation\n", (float) diff / n_lines);
    myfile << "Insertion Latency: " << (float) diff / n_lines << std::endl;
    uint64_t total_size = mdtrie->size();
    std::cout << "mdtrie storage: " << total_size << std::endl;
    myfile << "mdtrie storage: " << total_size << std::endl << "trie size: " << trie_size << " num trie nodes: " << num_trie_nodes << std::endl;
    myfile << "treeblock_nodes_size: " << treeblock_nodes_size << std::endl;
    myfile << "treeblock_frontier_size: " << treeblock_frontier_size << std::endl;
    myfile << "treeblock_primary_size: " << treeblock_primary_size << std::endl;
    myfile << "treeblock_primary_pointer_size: " << treeblock_primary_pointer_size << std::endl;
    myfile << "treeblock_variable_storage: " << treeblock_variable_storage << std::endl;
    myfile << "p_key_to_treeblock_compact: " << p_key_to_treeblock_compact_size << std::endl;
    myfile << "total_treeblock_num: " << total_treeblock_num << std::endl;
    myfile << "single_leaf_count: " << single_leaf_count << std::endl;

    myfile << "Without Primary key lookup support: " << total_size - treeblock_primary_size - treeblock_primary_pointer_size - p_key_to_treeblock_compact_size << std::endl;

    serialize_file.open("serialized.txt");
    uint64_t size = mdtrie->Serialize(serialize_file);
    std::cout << "size: " << size << std::endl;
    std::cout << "trie_node_serialized_size: " << trie_node_serialized_size << std::endl;
    std::cout << "blocks_serialized_size: " << blocks_serialized_size << std::endl;
    std::cout << "p_key_treeblock_compact_serialized_size: " << p_key_treeblock_compact_serialized_size << std::endl;
    std::cout << "primary_key_list_serialized_size: " << primary_key_list_serialized_size << std::endl;
    std::cout << "primary_key_ptr_vector_serialized_size: " << primary_key_ptr_vector_serialized_size << std::endl;
    // raise(SIGINT);
    exit(0);
/*
    tqdm bar2;
    TimeStamp check_diff = 0;
    
    for (uint64_t i = 0; i < n_lines; i++){
        bar2.progress(i, n_lines);
        auto check_point = (*all_points)[i];
        TimeStamp start = GetTimestamp();
        if (!mdtrie->check(&check_point)){
            raise(SIGINT);
            mdtrie->check(&check_point);
        } 
        check_diff += GetTimestamp() - start;  
    }
    bar2.finish();
    fprintf(stderr, "Average time to check one point: %f microseconds per operation\n", (float) check_diff / n_lines);

*/

    auto *start_range = new data_point();
    auto *end_range = new data_point();

/*
    int itr = 0;
    std::ofstream file("range_search_osm.csv");
    uint64_t search_volume = 1;
    srand(time(NULL));
    while (itr < 300){
        // if (itr % 20 == 0)
        //     std::cout << itr << std::endl;
        for (int j = 0; j < DIMENSION; j++){

            start_range->set_coordinate(j, min[j] + (max[j] - min[j] + 1) / 5 * (rand() % 5));
            end_range->set_coordinate(j, start_range->get_coordinate(j) + (max[j] - start_range->get_coordinate(j) + 1) / 5 * (rand() % 5));

            search_volume *= start_range->get_coordinate(j) - end_range->get_coordinate(j) + 1;
        }

        auto *found_points_temp = new point_array();
        start = GetTimestamp();
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points_temp);
        diff = GetTimestamp() - start;

        if (found_points_temp->size() > 0){
            // std::cout << "found: " << itr << std::endl;
            file << found_points_temp->size() << "," << diff << "," << search_volume << std::endl;
            itr ++;
        }
        search_volume = 1;
    }

*/

    for (dimension_t i = 0; i < DIMENSION; i++){
        start_range->set_coordinate(i, min[i]);
        end_range->set_coordinate(i, max[i]);
    }

    start = GetTimestamp();
    mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
    diff = GetTimestamp() - start;

    myfile << "found_pts size: " << found_points->size() << std::endl;
    myfile << "Range Search Latency: " << (float) diff / found_points->size() << std::endl;
    std::cout << "found_pts size: " << found_points->size() << std::endl;
    std::cout << "Range Search Latency: " << (float) diff / found_points->size() << std::endl;
    // std::cout << "add_primary_time: " << (float) add_primary_time / found_points->size() << std::endl;
    // std::cout << "diff: " << diff << " copy_vect_time: " << copy_vect_time << " update_symbol_time: " << update_symbol_time << " range_search_child_time: " << range_search_child_time << std::endl;
    // exit(0);

    return;  

}

data_point *profile_func(tree_block *parent_treeblock, node_t parent_node, symbol_t *node_path, level_t max_depth, symbol_t parent_symbol){

    parent_treeblock->get_node_path(parent_node, node_path); 

    node_path[max_depth - 1] = parent_symbol;
    
    return parent_treeblock->node_path_to_coordinates(node_path, DIMENSION);
}

void test_node_path_only(level_t max_depth, level_t trie_depth, preorder_t max_tree_node, std::vector<level_t> dimension_bits){
    
    dimension_to_num_bits = dimension_bits;
    create_level_to_num_children(dimension_bits, max_depth);
    auto *found_points = new point_array();
    auto *all_points = new std::vector<data_point>;

    insert_for_node_path(found_points, max_depth, trie_depth, max_tree_node, all_points);

    TimeStamp start;
    n_leaves_t found_points_size = found_points->size();
    TimeStamp diff_primary = 0;

    tqdm bar;
    n_leaves_t checked_points_size = 0;
    for (n_leaves_t i = 0; i < found_points_size; i+= 10){
        checked_points_size++;

        bar.progress(i, found_points_size);
        symbol_t *node_path = (symbol_t *)malloc((max_depth + 1) * sizeof(symbol_t));
        data_point *point = found_points->at(i);

        n_leaves_t returned_primary_key = point->read_primary();

        // data_point current_point = (*all_points)[returned_primary_key];

        // Lookup from primary key
        symbol_t *node_path_from_primary = (symbol_t *)malloc((max_depth + 1) * sizeof(symbol_t));

        
        tree_block *t_ptr = (tree_block *) (p_key_to_treeblock_compact.At(returned_primary_key));
        
        start = GetTimestamp();

        symbol_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(returned_primary_key, node_path_from_primary);
        node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

        data_point *returned_coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);
        // returned_coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);

        diff_primary += GetTimestamp() - start;

        for (dimension_t j = 0; j < DIMENSION; j++){
            if (returned_coordinates->get_coordinate(j) != point->get_coordinate(j)){
                raise(SIGINT);

                returned_coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);
            }
        }    

        free(node_path);    
        free(node_path_from_primary);
    }
    bar.finish();

    // fprintf(stderr, "Time per Checking: %f us, out of %ld points\n", (float)diff / checked_points_size, found_points->size());
    // fprintf(stderr, "Time per Primary Key lookup: %f us, out of %ld points\n", (float)diff_primary / checked_points_size, checked_points_size);
    myfile << "Lookup Latency: " << (float)diff_primary / checked_points_size << std::endl;
    std::cout << "Lookup Latency: " << (float)diff_primary / checked_points_size << std::endl;
     
}

void reset_values(){

    treeblock_nodes_size = 0;
    treeblock_frontier_size = 0;
    treeblock_primary_size = 0;
    treeblock_variable_storage = 0;
    p_key_to_treeblock_compact_size = 0;
    treeblock_primary_pointer_size = 0;
    total_treeblock_num = 0;
    single_leaf_count = 0;
    trie_size = 0;
    vector_size = 0;
    total_leaf_number = 0;
    treeblock_ptr_size = 0;    
}

int main() {

    // myfile.open("osm_latency_storage_tradeoff_loop.txt", std::ios_base::app);
    // myfile.open("osm_latency_storage_tradeoff_new.txt", std::ios_base::app);
    // myfile.open("osm_before_after.txt", std::ios_base::app);
    myfile.open("range search.txt");
    std::vector<level_t> dimension_bits = {8, 8, 8, 16, 32, 32};
    // std::vector<level_t> dimension_bits = {32, 32, 32, 32, 32, 32};
    TREEBLOCK_SIZE = 512;
    TRIE_DEPTH = 6;
    is_osm = true;

    myfile << "trie depth: " << TRIE_DEPTH << std::endl;
    myfile << "treeblock sizes: " << TREEBLOCK_SIZE << std::endl;
    test_node_path_only(32, TRIE_DEPTH, TREEBLOCK_SIZE, dimension_bits);
    myfile << std::endl;
    reset_values();      


    // TRIE_DEPTH = 2;
    // TREEBLOCK_SIZE = 256;
    // myfile << "trie depth: " << TRIE_DEPTH << std::endl;
    // myfile << "treeblock sizes: " << TREEBLOCK_SIZE << std::endl;
    // test_node_path_only(32, TRIE_DEPTH, TREEBLOCK_SIZE, dimension_bits);
    // myfile << std::endl;
    // reset_values();

    // exit(0);
    // ********************************************************

    // TRIE_DEPTH = 6;
    // TREEBLOCK_SIZE = 2048;
    // myfile << "trie depth: " << TRIE_DEPTH << std::endl;
    // myfile << "treeblock sizes: " << TREEBLOCK_SIZE << std::endl;
    // test_node_path_only(32, TRIE_DEPTH, TREEBLOCK_SIZE, dimension_bits);
    // myfile << std::endl;
    // reset_values();
 
    // ********************************************************

/*
    is_osm = true;
    bool skipped = true;
    for (TREEBLOCK_SIZE = 256; TREEBLOCK_SIZE <= 2048; TREEBLOCK_SIZE *= 2) // 3
    {  
        for (TRIE_DEPTH = 2; TRIE_DEPTH <= 8; TRIE_DEPTH += 2) // 4
        { 
            if (TRIE_DEPTH == 4 && TREEBLOCK_SIZE == 1024)
                skipped = false;

            if (skipped)
                continue;

            myfile << "trie depth: " << TRIE_DEPTH << std::endl;
            myfile << "treeblock sizes: " << TREEBLOCK_SIZE << std::endl;
            test_node_path_only(32, TRIE_DEPTH, TREEBLOCK_SIZE, dimension_bits);
            myfile << std::endl;
            reset_values();            
        }
    }

*/
    // is_osm = true;
    // TREEBLOCK_SIZE = 512;
    // for (TRIE_DEPTH = 2; TRIE_DEPTH <= 8; TRIE_DEPTH += 2) // 4
    // { 
    //     myfile << "trie depth: " << TRIE_DEPTH << std::endl;
    //     myfile << "treeblock sizes: " << TREEBLOCK_SIZE << std::endl;
    //     test_node_path_only(32, TRIE_DEPTH, TREEBLOCK_SIZE, dimension_bits);
    //     myfile << std::endl;
    //     reset_values();            
    // }    

    // myfile.close();

}