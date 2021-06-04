#include "catch.hpp"
#include "trie.h"

bool test_random_data(int n_points, int dimensions, level_type max_depth)
{
    symbol_type range = pow(2, max_depth);
    md_trie *mdtrie = new md_trie(dimensions);

    leaf_config *leaf_point = new leaf_config(dimensions);

    for (int itr = 1; itr <= n_points; itr ++){

        for (int i = 0; i < dimensions; i++){
            leaf_point->coordinates[i] = rand() % range;
        }
        mdtrie->insert_trie(leaf_point, max_depth);
        if (!mdtrie->check(leaf_point, max_depth)){
            return false;
        }
    }
    return true;
}

bool test_contiguous_data(int n_points, int dimensions, level_type max_depth)
{
    symbol_type range = pow(2, max_depth);
    md_trie *mdtrie = new md_trie(dimensions);

    int itr = 0;
    leaf_config *leaf_point = new leaf_config(dimensions);

    for (int itr = 1; itr <= n_points; itr ++){
        
        symbol_type first_half_value = rand() % range;
        for (int i = 0; i < dimensions / 2; i++){
            leaf_point->coordinates[i] = first_half_value;
        }
        symbol_type second_half_value = rand() % range;
        for (int i = dimensions / 2; i < dimensions; i++){
            leaf_point->coordinates[i] = second_half_value;
        }
        mdtrie->insert_trie(leaf_point, max_depth);
        if (!mdtrie->check(leaf_point, max_depth)){
            return false;
        }
    }
    return true;
}

bool test_nonexistent_data(int n_points, int dimensions, level_type max_depth)
{
    symbol_type range = pow(2, max_depth);
    md_trie *mdtrie = new md_trie(dimensions);

    int itr = 0;
    leaf_config *leaf_point = new leaf_config(dimensions);

    for (int itr = 1; itr <= n_points; itr ++){

        for (int i = 0; i < dimensions; i++){
            leaf_point->coordinates[i] = rand() % (range / 2);
        }
        mdtrie->insert_trie(leaf_point, max_depth);
    }
    for (int itr = 1; itr <= n_points; itr ++){

        for (int i = 0; i < dimensions; i++){
            leaf_point->coordinates[i] = rand() % (range / 2) + range / 2;
        }
        if (mdtrie->check(leaf_point, max_depth)){
            return false;
        }
    }
    return true;
}

TEST_CASE( "Check Random Data Insertion", "[trie]" ) {
    REQUIRE(test_random_data(50000, 10, 10));
}

TEST_CASE( "Check Nonexistent Data", "[trie]" ) {
    REQUIRE(test_nonexistent_data(10000, 10, 10));
}

TEST_CASE( "Check Contiguous Data", "[trie]" ) {
    REQUIRE(test_contiguous_data(10000, 10, 10));
}

