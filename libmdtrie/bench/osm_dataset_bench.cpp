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
            // if (i == 6 || i == 7){
                // leaf_point->set_coordinate(i, strtoul(token, &ptr, 10) / 10);
            // }
            // else {
            if (i < 8 - DIMENSION)
                continue;
            
            // if (i == 6 || i == 7){
            //     leaf_point->set_coordinate(i - (8 - DIMENSION), strtoul(token, &ptr, 10) % 100);
            // }
            // else {
            leaf_point->set_coordinate(i - (8 - DIMENSION), strtoul(token, &ptr, 10));
            // }

            // }
        }

        // if (leaf_point->get_coordinate(4) == 0)
        //     raise(SIGINT);

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
        // raise(SIGINT);
        (*all_points).push_back((*leaf_point));

        start = GetTimestamp();
        // if (leaf_point->get_coordinate(0) == 2 && leaf_point->get_coordinate(1) == 22 && leaf_point->get_coordinate(2) == 13 && leaf_point->get_coordinate(3) == 25 && leaf_point->get_coordinate(4) == 5 && leaf_point->get_coordinate(5) == 2021 && leaf_point->get_coordinate(6) == 247084021 && leaf_point->get_coordinate(7) == 603852889)
        //     raise(SIGINT);

        // if (n_points == 1050914)
        //     raise(SIGINT);

        mdtrie->insert_trie(leaf_point, max_depth, n_points);
        diff += GetTimestamp() - start;

        // if (!mdtrie->check(leaf_point, max_depth))
        //     raise(SIGINT);
        
        n_points ++;
    }

    for (dimension_t i = 0; i < DIMENSION; i++){
        if (max[i] == min[i]){
            raise(SIGINT);
        }
    }

    bar.finish();
    fprintf(stderr, "dimension: %d\n", DIMENSION);
    // fprintf(stderr, "md-trie size: %ld\n", mdtrie->size());   
    // fprintf(stderr, "top level trie size: %ld, primary key vector size: %ld, treeblock ptr size: %ld, treeblock_nodes_size: %ld\n", trie_size, vector_size, treeblock_ptr_size, treeblock_nodes_size);   
    fprintf(stderr, "Average time to insert one point: %f microseconds per insertion\n", (float) diff / n_points);
    
    // for (auto const &pair: node_children_to_occurrences) {
    //     std::cout << "{" << pair.first << ": " << pair.second << "}\n";
    // }

    // tqdm bar2;
    // for (uint64_t i = 0; i < n_lines; i++){
    //     bar2.progress(i, n_lines);
    //     auto check_point = (*all_points)[i];
    //     if (!mdtrie->check(&check_point, max_depth))
    //         raise(SIGINT);
    // }
    // bar2.finish();


    auto *start_range = new data_point(level_to_num_children[0]);
    auto *end_range = new data_point(level_to_num_children[0]);
    
    // std::vector<int> start_range_vect{ 0,19,7,2015,359921496,543371677 };
    // std::vector<int> end_range_vect{ 12,25,8,2019,1511670284,719557336 };

    // std::vector<int> start_range_vect{ 4,13,5,2009,1259725236,543371677 };
    // std::vector<int> end_range_vect{ 14,20,5,2009,1691631028,778285885 };

    std::vector<int> start_range_vect{ 0,1,1,2006,1259725236,543371677 };
    std::vector<int> end_range_vect{ 23,31,12,2021,1691631028,778285885 };

    for (dimension_t i = 0; i < DIMENSION; i++){
        start_range->set_coordinate(i, start_range_vect[i]);
        end_range->set_coordinate(i, end_range_vect[i]);
    }

    start = GetTimestamp();
    mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
    diff = GetTimestamp() - start;

    std::cout << "found_pts size: " << found_points->size() << " diff: " << diff << std::endl;
    std::cout << "update range latency: " << update_range_latency << std::endl;
    std::cout << "child latency " << child_latency << std::endl; 

    exit(0);

    int itr = 0;
    std::ofstream file("range_search_size_latency_osm_with_search_volume.csv", std::ios_base::app);
    uint64_t search_volume = 1;
    srand(time(NULL));
    while (itr < 1000){

        for (int j = 0; j < DIMENSION; j++){

            start_range->set_coordinate(j, min[j] + (max[j] - min[j] + 1) / 10 * (rand() % 10));
            end_range->set_coordinate(j, start_range->get_coordinate(j) + (max[j] - start_range->get_coordinate(j) + 1) / 10 * (rand() % 10));

            search_volume *= start_range->get_coordinate(j) - end_range->get_coordinate(j) + 1;
        }

        auto *found_points_temp = new point_array();
        start = GetTimestamp();
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points_temp);
        diff = GetTimestamp() - start;

        if (found_points_temp->size() > 0){
            std::cout << "found: " << itr << std::endl;
            file << found_points_temp->size() << "," << diff << "," << search_volume << std::endl;
            itr ++;
        }

        search_volume = 1;

    }

    fprintf(stderr, "Average time to range query one point: %f microseconds\n", (float) diff / found_points->size());
    fprintf(stderr, "found points: %ld, n points %ld\n", found_points->size(), n_points);
    std::cout << "Total Latency: " << diff << std::endl;
    // std::cout << "Top level trie: " << top_trie_range_search_latency << " treeblock latency: " << treeblock_range_search_latency << std::endl;
    std::cout << "Throughput: " << ((float) found_points->size() / diff) * 1000000 << std::endl;
    fprintf(stderr, "total time / total leaf nodes = %f\n", (float) diff / total_leaf_number);

}

data_point *profile_func(tree_block *parent_treeblock, node_t parent_node, symbol_t *node_path, level_t max_depth, symbol_t parent_symbol){

    parent_treeblock->get_node_path(parent_node, node_path); 

    node_path[max_depth - 1] = parent_symbol;
    
    return parent_treeblock->node_path_to_coordinates(node_path);
}

void test_node_path_only(level_t max_depth, level_t trie_depth, preorder_t max_tree_node){
    auto *found_points = new point_array();
    auto *all_points = new std::vector<data_point>(level_to_num_children[0]);

    insert_for_node_path(found_points, max_depth, trie_depth, max_tree_node, all_points);
    // assert(all_points->size() == 14583357);
    // while (found_points->size() == 0){
    //     found_points = insert_for_node_path(max_depth, trie_depth, max_tree_node);
    // }

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

        auto returned_coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary);

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
    fprintf(stderr, "Time per Primary Key lookup: %f us, out of %ld points\n", (float)diff_primary / checked_points_size, found_points->size());
    fprintf(stderr, "Time spent on primary key: %f\n", (float)primary_time / checked_points_size);
     
}

int main() {
    is_osm = true;
    test_node_path_only(32, 10, 512);
}