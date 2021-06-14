#include "catch.hpp"
#include "trie.h"

bool test_range_search(int n_points, int dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_nodes, int n_itr = 50, int n_checked_points = 100000)
{
    auto range = (symbol_type)pow(2, max_depth);
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_nodes);
    auto *leaf_point = new leaf_config(dimensions);

    for (int itr = 1; itr <= n_points; itr ++){
        for (int i = 0; i < dimensions; i++){
            leaf_point->coordinates[i] = rand() % range;
        }
        mdtrie->insert_trie(leaf_point, max_depth);
    }
    auto *start_range = new leaf_config(dimensions);
    auto *end_range = new leaf_config(dimensions);
    auto *found_points = new leaf_array();

    for (int itr = 1; itr <= n_itr; itr++){
        for (int i = 0; i < dimensions; i++){
            start_range->coordinates[i] = rand() % range;
            end_range->coordinates[i] = rand() % range;
            while (end_range->coordinates[i] < start_range->coordinates[i]){
                end_range->coordinates[i] = rand() % range;
            }
        }

        mdtrie->range_search_trie(start_range, end_range, mdtrie->get_root(), 0, found_points);
        
        for (int i = 0; i < found_points->n_points; i++){
            leaf_config *leaf = found_points->points[i];
            if (!mdtrie->check(leaf, max_depth)){
                return false;
            }
        }

        int checked_points = 0;
        while (checked_points <= n_checked_points){
            auto *leaf_check = new leaf_config(dimensions);
            for (int i = 0; i < dimensions; i++){
                point_type coordinate = rand() % range;
                while (coordinate < start_range->coordinates[i] || coordinate > end_range->coordinates[i]){
                    coordinate = rand() % range;
                }
                leaf_check->coordinates[i] = coordinate;
            }
            bool found = false;
            for (int q = 0; q < found_points->n_points; q++){
                leaf_config *found_leaf = found_points->points[q];
                found = true;
                for (int i = 0; i < dimensions; i++){
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
    REQUIRE(test_range_search(5000, 8, 10, 3, 128, 50, 10000));
}