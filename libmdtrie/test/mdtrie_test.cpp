#include "catch.hpp"
#include "trie.h"

bool test_random_data(n_leaves_type n_points, dimension_type dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_nodes)
{
    auto range = (symbol_type)pow(2, max_depth);
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_nodes);
    auto *leaf_point = new leaf_config(dimensions);

    for (n_leaves_type itr = 1; itr <= n_points; itr ++){
        for (dimension_type i = 0; i < dimensions; i++){
            leaf_point->coordinates[i] = (point_type)rand() % range;
        }
        mdtrie->insert_trie(leaf_point, max_depth);
        if (!mdtrie->check(leaf_point, max_depth)){
            return false;
        }
    }
    return true;
}

bool test_contiguous_data(n_leaves_type n_points, dimension_type dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_node)
{
    auto range = (symbol_type)pow(2, max_depth);
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new leaf_config(dimensions);

    for (n_leaves_type itr = 1; itr <= n_points; itr ++){
        
        auto first_half_value = (symbol_type)rand() % range;
        for (dimension_type i = 0; i < dimensions / 2; i++){
            leaf_point->coordinates[i] = first_half_value;
        }
        auto second_half_value = (symbol_type)rand() % range;
        for (dimension_type i = dimensions / 2; i < dimensions; i++){
            leaf_point->coordinates[i] = second_half_value;
        }
        mdtrie->insert_trie(leaf_point, max_depth);
        if (!mdtrie->check(leaf_point, max_depth)){
            return false;
        }
    }
    return true;
}

bool test_nonexistent_data(n_leaves_type n_points, dimension_type dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_node)
{
    auto range = (symbol_type)pow(2, max_depth);
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new leaf_config(dimensions);

    for (n_leaves_type itr = 1; itr <= n_points; itr ++){
        for (dimension_type i = 0; i < dimensions; i++){
            leaf_point->coordinates[i] = rand() % (range / 2);
        }
        mdtrie->insert_trie(leaf_point, max_depth);
    }
    for (n_leaves_type itr = 1; itr <= n_points; itr ++){
        for (dimension_type i = 0; i < dimensions; i++){
            leaf_point->coordinates[i] = rand() % (range / 2) + range / 2;
        }
        if (mdtrie->check(leaf_point, max_depth)){
            return false;
        }
    }
    return true;
}

//  n_points, dimensions, max_depth, trie_depth, max_tree_node
 TEST_CASE( "Check Random Data Insertion", "[trie]" ) {
     REQUIRE(test_random_data(100000, 8, 20, 10, 1024));
 }

 TEST_CASE( "Check Nonexistent Data", "[trie]" ) {
     REQUIRE(test_nonexistent_data(10000, 10, 10, 3, 128));
 }

 TEST_CASE( "Check Contiguous Data", "[trie]" ) {
     REQUIRE(test_contiguous_data(10000, 10, 10, 3, 128));
 }

