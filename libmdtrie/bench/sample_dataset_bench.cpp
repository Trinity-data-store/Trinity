#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <tqdm.h>
#include <vector>

// Last coordinates all 0
// Second last coordinate > 32
const int DIMENSION = 6; // Max: 6
const symbol_t NUM_BRANCHES = pow(2, DIMENSION);
FILE *fptr;
char file_path[] = "benchmark_range_search_2.csv";


void insert_for_node_path(point_array<DIMENSION> *found_points, level_t max_depth, level_t trie_depth, preorder_t max_tree_node, std::vector<data_point<DIMENSION>> *all_points){
    // to-do
    
    auto *mdtrie = new md_trie<DIMENSION, NUM_BRANCHES>(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point<DIMENSION>();

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
        mdtrie->insert_trie(leaf_point, max_depth);
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
    
    auto *start_range = new data_point<DIMENSION>();
    auto *end_range = new data_point<DIMENSION>();

    for (dimension_t i = 0; i < DIMENSION; i++){
        start_range->set_coordinate(i, min[i]);
        end_range->set_coordinate(i, max[i]);
    }
    start = GetTimestamp();
    mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
    diff = GetTimestamp() - start;

    fprintf(stderr, "Average time to range query one point: %f microseconds\n", (float) diff / found_points->size());
    fprintf(stderr, "found points: %ld, n points %ld, current_primary_key: %ld\n", found_points->size(), n_points, current_primary_key);
    fprintf(stderr, "total time / total leaf nodes = %f\n", (float) diff / total_leaf_number);

}

data_point<DIMENSION> * profile_func(tree_block<DIMENSION, NUM_BRANCHES> *parent_treeblock, node_t parent_node, symbol_t *node_path, level_t max_depth, symbol_t parent_symbol){
    parent_treeblock->get_node_path(parent_node, node_path); 

    node_path[max_depth - 1] = parent_symbol;
    
    return parent_treeblock->node_path_to_coordinates(node_path);
}

void test_node_path_only(level_t max_depth, level_t trie_depth, preorder_t max_tree_node){
    auto *found_points = new point_array<DIMENSION>();
    auto *all_points = new std::vector<data_point<DIMENSION>>();

    insert_for_node_path(found_points, max_depth, trie_depth, max_tree_node, all_points);
    // assert(all_points->size() == 14583357);
    // while (found_points->size() == 0){
    //     found_points = insert_for_node_path(max_depth, trie_depth, max_tree_node);
    // }

    TimeStamp diff = 0;
    TimeStamp start;
    n_leaves_t found_points_size = found_points->size();
    TimeStamp diff_primary = 0;

    tqdm bar;
    n_leaves_t checked_points_size = 0;
    for (n_leaves_t i = 0; i < found_points_size; i+= 10){
        checked_points_size++;
        bar.progress(i, found_points_size);
        symbol_t *node_path = (symbol_t *)malloc((max_depth + 1) * sizeof(symbol_t));
        data_point<DIMENSION> *point = found_points->at(i);
        tree_block<DIMENSION, NUM_BRANCHES> *parent_treeblock = point->get_parent_treeblock();
        symbol_t parent_symbol = point->get_parent_symbol();
        node_t parent_node = point->get_parent_node();

        start = GetTimestamp();
        // parent_treeblock->get_node_path(parent_node, node_path); 

        // node_path[max_depth - 1] = parent_symbol;
        
        // auto returned_coordinates = parent_treeblock->node_path_to_coordinates(node_path);
        auto returned_coordinates = profile_func(parent_treeblock, parent_node, node_path, max_depth, parent_symbol);
        diff += GetTimestamp() - start;

        for (dimension_t j = 0; j < DIMENSION; j++){
            if (returned_coordinates->get_coordinate(j) != point->get_coordinate(j)){
                raise(SIGINT);
            }
        }


        n_leaves_t returned_primary_key = point->read_primary();
        bool found = false;

        data_point<DIMENSION> current_point = (*all_points)[returned_primary_key];

        for (dimension_t j = 0; j < DIMENSION; j++){
            if (returned_coordinates->get_coordinate(j) != current_point.get_coordinate(j)){
                break;
            }
            if (j == DIMENSION - 1){
                found = true;
            }
        }

        if (!found){
            raise(SIGINT);
        }

        // Lookup from primary key

        symbol_t *node_path_from_primary = (symbol_t *)malloc((max_depth + 1) * sizeof(symbol_t));

        
        tree_block<DIMENSION, NUM_BRANCHES> *t_ptr = (tree_block<DIMENSION, NUM_BRANCHES> *) (p_key_to_treeblock_compact.At(returned_primary_key));
        
        start = GetTimestamp();
        symbol_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(returned_primary_key, node_path_from_primary);
        node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

        returned_coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary);

        // returned_coordinates = profile_func(t_ptr, returned_primary_key, node_path_from_primary, max_depth);
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

    test_node_path_only(32, 10, 512);
}