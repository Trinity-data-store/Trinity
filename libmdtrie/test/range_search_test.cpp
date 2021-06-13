#include "catch.hpp"
#include "trie.h"

bool test_range_search(int n_points, int dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_nodes){

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

    for (int itr = 1; itr <= 50; itr++){
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
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root_, 0, found_points);
        
        // Check if these are the only points;
        for (int i = 0; i < found_points->n_points; i++){
            leaf_config *leaf = found_points->points[i];
            if (!mdtrie->check(leaf, max_depth)){
                fprintf(stderr, "Error, points not found!\n");
                return false;
            }
        }

        for (point_type i = start_range->coordinates[0]; i <= end_range->coordinates[0]; i++){
            for (point_type j = start_range->coordinates[1]; j <= end_range->coordinates[1]; j++){
                for (point_type k = start_range->coordinates[2]; k <= end_range->coordinates[2]; k++){
                    
                    bool present = false;
                    for (int q = 0; q < found_points->n_points; q++){
                        leaf_config *leaf = found_points->points[q];
                        if (leaf->coordinates[0] == i && leaf->coordinates[1] == j && leaf->coordinates[2] == k){
                            present = true;
                            break;
                        }
                    }
                    if (present){
                        continue;
                    }
                    auto *leaf_check = new leaf_config(dimensions);
                    leaf_check->coordinates[0] = i;
                    leaf_check->coordinates[1] = j;
                    leaf_check->coordinates[2] = k;
                    if (mdtrie->check(leaf_check, max_depth)){
                        fprintf(stderr, "Error, other points are found!\n");
                        return false;
                    }                    
                }
            }            
        }


        // Free
        found_points->n_points = 0;
        free(found_points->points);
        found_points->points = (leaf_config **)malloc(sizeof(leaf_config *));
    }    
    return true;

}

TEST_CASE( "Test", "[trie]" ) {
    REQUIRE(test_range_search(1000, 3, 10, 3, 128));
    
}