#include "catch.hpp"
#include "trie.h"

const int DIMENSION = 2;
const symbol_t NUM_BRANCHES = pow(2, DIMENSION);

void insert_for_node_path(point_array<DIMENSION> *found_points, level_t max_depth, level_t trie_depth, preorder_t max_tree_node, std::vector<data_point<DIMENSION>> *all_points){
    
    auto *mdtrie = new md_trie<DIMENSION, NUM_BRANCHES>(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point<DIMENSION>();

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;

    FILE *fp = fopen("/home/ziming/md-trie/libmdtrie/bench/data/sample_shuf.txt", "r");

    if (fp == nullptr)
    {
        fprintf(stderr, "file not found\n");
        exit(EXIT_FAILURE);
    }
    
    n_leaves_t n_points = 0;
    uint64_t max[DIMENSION];
    uint64_t min[DIMENSION];
    n_leaves_t n_lines = 14583357;

    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *token = strtok(line, " ");
        char *ptr;
        for (uint8_t i = 0; i < 2; i ++){
            token = strtok(nullptr, " ");
        }
        for (dimension_t i = 0; i < DIMENSION; i++){
            if (i >= 4){
                leaf_point->set_coordinate(i, leaf_point->get_coordinate(i % 4));
            }
            else {
                token = strtok(nullptr, " ");
                leaf_point->set_coordinate(i, strtoul(token, &ptr, 10));
            }
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
        // all_stored_points.push_back((*leaf_point));

        mdtrie->insert_trie(leaf_point, max_depth);
        n_points ++;
        if (n_points > 1000000){
            break;
        }
    }

    

    auto *start_range = new data_point<DIMENSION>();
    auto *end_range = new data_point<DIMENSION>();

    for (dimension_t i = 0; i < DIMENSION; i++){
        start_range->set_coordinate(i,  min[i]);
        end_range->set_coordinate(i, max[i]);
    }
    mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
    if (found_points->size() != n_points){
        raise(SIGINT);
    }
    fclose(fp);
}

bool test_lookup(level_t max_depth, level_t trie_depth, preorder_t max_tree_node){
    auto *found_points = new point_array<DIMENSION>();
    auto *all_points = new std::vector<data_point<DIMENSION>>();
    // 
    // points = all_points;
    insert_for_node_path(found_points, max_depth, trie_depth, max_tree_node, all_points);
    // raise(SIGINT);
    // if (found_points->size() != 14583357){
    //     return false;
    // }

    TimeStamp diff = 0;
    TimeStamp start;
    n_leaves_t found_points_size = found_points->size();
    TimeStamp diff_primary = 0;

    for (n_leaves_t i = 0; i < found_points_size; i++){
        
        symbol_t *node_path = (symbol_t *)malloc((max_depth + 1) * sizeof(symbol_t));
        data_point<DIMENSION> *point = found_points->at(i);

        /**
            Test lookup given (treeblock pointer, immediate parent, symbol)
            return if coordinates match
        */

        tree_block<DIMENSION, NUM_BRANCHES> *parent_treeblock = point->get_parent_treeblock();
        symbol_t parent_symbol = point->get_parent_symbol();
        node_t parent_node = point->get_parent_node();

        parent_treeblock->get_node_path(parent_node, node_path); 
        node_path[max_depth - 1] = parent_symbol;
        auto returned_coordinates = parent_treeblock->node_path_to_coordinates(node_path);

        for (dimension_t j = 0; j < DIMENSION; j++){
            if (returned_coordinates->get_coordinate(j) != point->get_coordinate(j)){
                return false;
            }
        }

        /**
            Test if primary keys of returned points are correct
            Range search returns coordinates and their associated primary keys
            This test goes back to the dataset to check if the two match
        */

        preorder_t returned_primary_key = point->read_primary();
        // bool found = false;

        data_point<DIMENSION> current_point = (*all_points)[returned_primary_key];

        for (dimension_t j = 0; j < DIMENSION; j++){
            if (returned_coordinates->get_coordinate(j) != current_point.get_coordinate(j)){
                return false;
            }
            // if (j == DIMENSION - 1){
            //     found = true;
            // }
        }

        // if (!found)
        //     return false;

        /**
            Test md-trie lookup given primary key
            Range search finds primary keys of all found points
            This test lookup the coordinates matching these found points
            and check if coordinates match with the returned points
        */

        symbol_t *node_path_from_primary = (symbol_t *)malloc((max_depth + 1) * sizeof(symbol_t));

        tree_block<DIMENSION, NUM_BRANCHES> *t_ptr = (tree_block<DIMENSION, NUM_BRANCHES> *)p_key_to_treeblock[returned_primary_key];
        
        symbol_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(returned_primary_key, node_path_from_primary);
        node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

        returned_coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary);

        for (dimension_t j = 0; j < DIMENSION; j++){
            if (returned_coordinates->get_coordinate(j) != point->get_coordinate(j)){
                return false;
            }
        }    

        free(node_path);    
        free(node_path_from_primary);
    }
    return true;
}

TEST_CASE("Check Lookup", "[trie]") {

    bool result = test_lookup(32, 10, 1024);
    REQUIRE(result);
}