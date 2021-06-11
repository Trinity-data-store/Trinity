#include "catch.hpp"
#include "trie.h"

bool test_range_search(int n_points, int dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_nodes){

    symbol_type range = pow(2, max_depth);
    md_trie *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_nodes);

    leaf_config *leaf_point = new leaf_config(dimensions);

    for (int itr = 1; itr <= n_points; itr ++){

        for (int i = 0; i < dimensions; i++){
            leaf_point->coordinates[i] = rand() % range;
        }
        mdtrie->insert_trie(leaf_point, max_depth);
    }
    leaf_config *start_range = new leaf_config(dimensions);
    leaf_config *end_range = new leaf_config(dimensions);


    for (int itr = 1; itr <= 1; itr++){
        for (int i = 0; i < dimensions; i++){
            start_range->coordinates[i] = rand() % range;
            end_range->coordinates[i] = rand() % range;
            while (end_range->coordinates[i] < start_range->coordinates[i]){
                end_range->coordinates[i] = rand() % range;
            }
        }
        printf("Range search: \n");
        for (int i = 1; i <= dimensions; i++){
            printf("%dth dimension - [%ld, %ld]\n", i, start_range->coordinates[i-1], end_range->coordinates[i-1]);
        }
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root_, 0);
    }    
    return true;

}

TEST_CASE( "Test", "[trie]" ) {
    REQUIRE(test_range_search(500, 3, 10, 3, 128));
    
}