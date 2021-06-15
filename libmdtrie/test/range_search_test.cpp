#include "catch.hpp"
#include "trie.h"

bool test_range_search(n_leaves_type n_points, dimension_type dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_nodes, uint32_t n_itr = 50, n_leaves_type n_checked_points = 100000)
{
    auto range = (symbol_type)pow(2, max_depth);
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_nodes);
    auto *leaf_point = new leaf_config(dimensions);

    for (n_leaves_type itr = 1; itr <= n_points; itr ++){
        for (dimension_type i = 0; i < dimensions; i++){
            leaf_point->coordinates[i] = rand() % range;
        }
        mdtrie->insert_trie(leaf_point, max_depth);
    }
    auto *start_range = new leaf_config(dimensions);
    auto *end_range = new leaf_config(dimensions);
    auto *found_points = new leaf_array();
    uint8_t *representation = (uint8_t *)malloc(sizeof(uint8_t) * dimensions);

    for (uint32_t itr = 1; itr <= n_itr; itr++){
        for (dimension_type i = 0; i < dimensions; i++){
            start_range->coordinates[i] = rand() % range;
            end_range->coordinates[i] = rand() % range;
            while (end_range->coordinates[i] < start_range->coordinates[i]){
                end_range->coordinates[i] = rand() % range;
            }
        }

        mdtrie->range_search_trie(start_range, end_range, mdtrie->get_root(), 0, found_points, representation);
        
        for (n_leaves_type i = 0; i < found_points->n_points; i++){
            leaf_config *leaf = found_points->points[i];
            if (!mdtrie->check(leaf, max_depth)){
                return false;
            }
        }

        n_leaves_type checked_points = 0;
        while (checked_points <= n_checked_points){
            auto *leaf_check = new leaf_config(dimensions);
            for (dimension_type i = 0; i < dimensions; i++){
                point_type coordinate = rand() % range;
                while (coordinate < start_range->coordinates[i] || coordinate > end_range->coordinates[i]){
                    coordinate = rand() % range;
                }
                leaf_check->coordinates[i] = coordinate;
            }
            bool found = false;
            for (n_leaves_type q = 0; q < found_points->n_points; q++){
                leaf_config *found_leaf = found_points->points[q];
                found = true;
                for (dimension_type i = 0; i < dimensions; i++){
                    if (found_leaf->coordinates[i] != leaf_check->coordinates[i]){
                        found = false;
                        break;
                    }
                }
                if (found)
                    break;
            }
            if (!found){
                checked_points ++;
                if (mdtrie->check(leaf_check, max_depth))
                    return false;
            }
            else {
                if (!mdtrie->check(leaf_check, max_depth))
                    return false;
            }
        }
        found_points->reset();
    }    
    return true;
}

// int n_points, int dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_nodes, int n_itr, int n_checked_points
TEST_CASE( "Test", "Range Search Trie" ) {
    srand(static_cast<unsigned int>(time(0)));
    REQUIRE(test_range_search(5000, 8, 10, 3, 128, 100, 10000));
}