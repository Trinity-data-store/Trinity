#include "catch.hpp"
#include "trie.h"

const int DIMENSION = 10;

bool test_random_data(n_leaves_t n_points, level_t max_depth, level_t trie_depth,
                      preorder_t max_tree_nodes) {

    /**
        Insert random points into the mdtrie
        Check whether the inserted points exist
    */     

    bitmap::CompactPtrVector tmp_ptr_vect(n_points);
    p_key_to_treeblock_compact = &tmp_ptr_vect;
    create_level_to_num_children(std::vector<level_t>(DIMENSION, max_depth), std::vector<level_t>(DIMENSION, 0), max_depth);
    auto range = (n_leaves_t) pow(2, max_depth);
    auto *mdtrie = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_nodes);
    data_point<DIMENSION> leaf_point;

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {
        for (dimension_t i = 0; i < DIMENSION; i++) {
            leaf_point.set_coordinate(i, (point_t) rand() % range);
        }
        mdtrie->insert_trie(&leaf_point, itr - 1, p_key_to_treeblock_compact);
        if (!mdtrie->check(&leaf_point)) {
            return false;
        }
    }
    delete mdtrie;
    return true;
}

bool test_nonexistent_data(n_leaves_t n_points, level_t max_depth, level_t trie_depth,
                           preorder_t max_tree_node) {

    /**
        Insert random points into the mdtrie
        Query non-existent points
        Pass the test if none of those points can be found
    */ 

    bitmap::CompactPtrVector tmp_ptr_vect(n_points);
    p_key_to_treeblock_compact = &tmp_ptr_vect;
    create_level_to_num_children(std::vector<level_t>(DIMENSION, max_depth), std::vector<level_t>(DIMENSION, 0), max_depth);
    auto range = (n_leaves_t) pow(2, max_depth);
    auto *mdtrie = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
    data_point<DIMENSION> leaf_point;

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {
        for (dimension_t i = 0; i < DIMENSION; i++) {
            leaf_point.set_coordinate(i, rand() % (range / 2));
        }
        mdtrie->insert_trie(&leaf_point, itr - 1, p_key_to_treeblock_compact);
    }
    for (n_leaves_t itr = 1; itr <= n_points; itr++) {
        for (dimension_t i = 0; i < DIMENSION; i++) {
            leaf_point.set_coordinate(i, rand() % (range / 2) + range / 2);
        }
        if (mdtrie->check(&leaf_point)) {
            return false;
        }
    }
    delete mdtrie;
    return true;
}

bool test_contiguous_data(n_leaves_t n_points, level_t max_depth, level_t trie_depth,
                          preorder_t max_tree_node) {

    /**
        Insert random points that are close to each other into the mdtrie
        Check whether the inserted points exist
    */ 
   
    bitmap::CompactPtrVector tmp_ptr_vect(n_points);
    p_key_to_treeblock_compact = &tmp_ptr_vect;
    create_level_to_num_children(std::vector<level_t>(DIMENSION, max_depth), std::vector<level_t>(DIMENSION, 0), max_depth);
    auto range = (n_leaves_t) pow(2, max_depth);
    auto *mdtrie = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
    data_point<DIMENSION> leaf_point;

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {

        auto first_half_value = (point_t) rand() % range;
        for (dimension_t i = 0; i < DIMENSION / 2; i++) {
            leaf_point.set_coordinate(i, first_half_value);
        }
        auto second_half_value = (point_t) rand() % range;
        for (dimension_t i = DIMENSION / 2; i < DIMENSION; i++) {
            leaf_point.set_coordinate(i, second_half_value);
        }
        mdtrie->insert_trie(&leaf_point, itr - 1, p_key_to_treeblock_compact);
        if (!mdtrie->check(&leaf_point)) {
            raise(SIGINT);
            return false;
        }
    }
    return true;
}

TEST_CASE("Check Random Data Insertion", "[trie]") {
    REQUIRE(test_random_data(10000, 20, 10, 1024));
}

TEST_CASE("Check Nonexistent Data", "[trie]") {
    REQUIRE(test_nonexistent_data(50000, 20, 10, 1024));
}

TEST_CASE("Check Contiguous Data", "[trie]") {
    REQUIRE(test_contiguous_data(50000, 10, 3, 128));
}
