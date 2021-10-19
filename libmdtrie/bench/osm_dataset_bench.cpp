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
const symbol_t NUM_BRANCHES = pow(2, DIMENSION);


void insert_for_node_path(point_array *found_points, level_t max_depth, level_t trie_depth, preorder_t max_tree_node, std::vector<data_point> *all_points){
    // to-do
    
    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point(level_to_num_children[0]);

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
    // TimeStamp start, diff;
    
    n_leaves_t n_points = 0;
    uint64_t max[DIMENSION];
    uint64_t min[DIMENSION];
    n_leaves_t n_lines = 14252681;
    total_points_count = n_lines;
    // diff = 0;
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
            
            // std::string token_string(token);
            // std::reverse(token_string.begin(), token_string.end());

            // uint64_t coordinate = std::stoull(token_string);
            // leaf_point->set_coordinate(i - (8 - DIMENSION), coordinate);
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

        // if (n_points == 4868)
        //     raise(SIGINT);

        mdtrie->insert_trie(leaf_point, n_points);
        diff += GetTimestamp() - start;

        // if (!mdtrie->check(leaf_point)){
        //     raise(SIGINT);
        //     mdtrie->check(leaf_point);
        // }   

        n_points ++;
    }

    bar.finish();
    fprintf(stderr, "dimension: %d\n", DIMENSION);
    fprintf(stderr, "Average time to insert one point: %f microseconds per operation\n", (float) diff / n_lines);
    std::cout << "mdtrie storage: " << mdtrie->size() << " trie size" << trie_size << "num trie nodes" << num_trie_nodes << std::endl;

    // raise(SIGINT);
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


    auto *start_range = new data_point(DIMENSION);
    auto *end_range = new data_point(DIMENSION);

    int itr = 0;
    std::ofstream file("range_search_osm.csv");
    uint64_t search_volume = 1;
    srand(time(NULL));
    while (itr < 300){
        if (itr % 20 == 0)
            std::cout << itr << std::endl;
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



    for (dimension_t i = 0; i < DIMENSION; i++){
        start_range->set_coordinate(i, min[i]);
        end_range->set_coordinate(i, max[i]);
    }

    start = GetTimestamp();
    mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
    diff = GetTimestamp() - start;

    std::cout << "found_pts size: " << found_points->size() << " diff: " << diff << std::endl;

    // raise(SIGINT);
    // mdtrie->check(found_points->at(6860));
    return;  

}

data_point *profile_func(tree_block *parent_treeblock, node_t parent_node, symbol_t *node_path, level_t max_depth, symbol_t parent_symbol){

    parent_treeblock->get_node_path(parent_node, node_path); 

    node_path[max_depth - 1] = parent_symbol;
    
    return parent_treeblock->node_path_to_coordinates(node_path, DIMENSION);
}

void test_node_path_only(level_t max_depth, level_t trie_depth, preorder_t max_tree_node){

    // std::vector<level_t> dimension_bits = {32, 32, 32, 32, 32, 32};
    std::vector<level_t> dimension_bits = {6, 6, 6, 12, 32, 32};
    // std::vector<level_t> dimension_bits = {5, 5, 4, 11, 32, 32}; //TODO: fix BUG in lookup given primary key
    
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

        data_point current_point = (*all_points)[returned_primary_key];

        // Lookup from primary key
        symbol_t *node_path_from_primary = (symbol_t *)malloc((max_depth + 1) * sizeof(symbol_t));

        
        tree_block *t_ptr = (tree_block *) (p_key_to_treeblock_compact.At(returned_primary_key));
        
        start = GetTimestamp();
        symbol_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(returned_primary_key, node_path_from_primary);
        node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

        auto returned_coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);

        diff_primary += GetTimestamp() - start;

        for (dimension_t j = 0; j < DIMENSION; j++){
            if (returned_coordinates->get_coordinate(j) != point->get_coordinate(j)){
                raise(SIGINT);
            }
        }    

        free(node_path);    
        free(node_path_from_primary);
    }
    bar.finish();

    // fprintf(stderr, "Time per Checking: %f us, out of %ld points\n", (float)diff / checked_points_size, found_points->size());
    fprintf(stderr, "Time per Primary Key lookup: %f us, out of %ld points\n", (float)diff_primary / checked_points_size, checked_points_size);
     
}

int main() {
    is_osm = true;
    test_node_path_only(32, 10, 512);
}