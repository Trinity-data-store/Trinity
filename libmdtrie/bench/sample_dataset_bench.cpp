#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <tqdm.h>
#include <vector>
#include <iostream>
#include <fstream>

// Last coordinates all 0
// Second last coordinate > 32
const int DIMENSION = 6; // Max: 6
const symbol_t NUM_BRANCHES = pow(2, DIMENSION);
FILE *fptr;
char file_path[] = "benchmark_range_search_2.csv";


void insert_for_node_path(point_array *found_points, level_t max_depth, level_t trie_depth, preorder_t max_tree_node, std::vector<data_point> *all_points){
    // to-do
    
    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point();

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../libmdtrie/bench/data/sample_shuf.txt", "r");

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
    n_leaves_t n_lines = 14583357;
    // diff = 0;
    tqdm bar;
    TimeStamp start, diff;
    diff = 0;

    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar.progress(n_points, n_lines);
        char *token = strtok(line, " ");
        char *ptr;

        // token = strtok(nullptr, " ");
        // leaf_point->set_coordinate(0, strtoul(token, &ptr, 10));

        for (uint8_t i = 1; i <= 2; i ++){
            token = strtok(nullptr, " ");
        }

        for (dimension_t i = 0; i < DIMENSION; i++){
            // One dimension with > 32
            // if (i == 4)
            //     token = strtok(nullptr, " ");
                
            token = strtok(nullptr, " ");
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
        // raise(SIGINT);
        (*all_points).push_back((*leaf_point));

        start = GetTimestamp();
        mdtrie->insert_trie(leaf_point, max_depth, n_points);
        diff += GetTimestamp() - start;
        n_points ++;
    }

    for (dimension_t i = 0; i < DIMENSION; i++){
        if (max[i] == min[i]){
            raise(SIGINT);
        }
    }

    bar.finish();
    fprintf(stderr, "dimension: %d\n", DIMENSION);
    fprintf(stderr, "md-trie size: %ld\n", mdtrie->size());   
    fprintf(stderr, "top level trie size: %ld, primary key vector size: %ld, treeblock ptr size: %ld\n", trie_size, vector_size, treeblock_ptr_size);   
    fprintf(stderr, "Average time to insert one point: %f microseconds per insertion\n", (float) diff / n_points);

    for (auto const &pair: node_children_to_occurrences) {
        std::cout << "{" << pair.first << ": " << pair.second << "}\n";
    }

    // tqdm bar2;
    // for (uint64_t i = 0; i < n_lines; i++){
    //     bar2.progress(i, n_lines);
    //     auto check_point = (*all_points)[i];
    //     if (!mdtrie->check(&check_point, max_depth))
    //         raise(SIGINT);
    // }
    // bar2.finish();
    
    auto *start_range = new data_point();
    auto *end_range = new data_point();

    // for (dimension_t i = 0; i < DIMENSION; i++){
    //     start_range->set_coordinate(i, min[i]);
    //     end_range->set_coordinate(i, max[i]);
    // }
    // start = GetTimestamp();
    // mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
    // diff = GetTimestamp() - start;

    int itr = 0;
    std::ofstream file("range_search_size_latency_filesystem.csv", std::ios_base::app);
    uint64_t search_volume = 1;

    while (itr < 1000){

        for (int j = 0; j < DIMENSION; j++){

            start_range->set_coordinate(j, min[j] + (max[j] - min[j] + 1) / 15 * (rand() % 15));
            end_range->set_coordinate(j, start_range->get_coordinate(j) + (max[j] - start_range->get_coordinate(j) + 1) / 15 * (rand() % 15));

            search_volume *= start_range->get_coordinate(j) - end_range->get_coordinate(j) + 1;

        }

        auto *found_points_temp = new point_array();

        start = GetTimestamp();
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points_temp);
        diff = GetTimestamp() - start;

        if (found_points_temp->size() > 1000){
            std::cout << "found: " << itr << std::endl;
            file << found_points_temp->size() << "," << diff << "," << search_volume << std::endl;
            itr ++;
        }

        search_volume = 1;

    }

    fprintf(stderr, "Average time to range query one point: %f microseconds\n", (float) diff / found_points->size());
    fprintf(stderr, "found points: %ld, n points %ld\n", found_points->size(), n_points);
    fprintf(stderr, "total time / total leaf nodes = %f\n", (float) diff / total_leaf_number);

}

data_point * profile_func(tree_block *parent_treeblock, node_t parent_node, symbol_t *node_path, level_t max_depth, symbol_t parent_symbol){
    parent_treeblock->get_node_path(parent_node, node_path); 

    node_path[max_depth - 1] = parent_symbol;
    
    return parent_treeblock->node_path_to_coordinates(node_path);
}

void test_node_path_only(level_t max_depth, level_t trie_depth, preorder_t max_tree_node){
    auto *found_points = new point_array();
    auto *all_points = new std::vector<data_point>();

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


    fprintf(stderr, "Time per Primary Key lookup: %f us, out of %ld points\n", (float)diff_primary / checked_points_size, found_points->size());
    fprintf(stderr, "Time spent on primary key: %f\n", (float)primary_time / checked_points_size);
     
}

int main() {

    test_node_path_only(32, 10, 512);
}