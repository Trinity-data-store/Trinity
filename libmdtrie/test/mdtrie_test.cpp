#include "catch.hpp"
#include "trie.h"

const int DIMENSION_RANDOM = 8;
const int DIMENSION_NONEXISTENT = 10;
const int DIMENSION_CONTIGUOUS = 10;

bool test_random_data(n_leaves_t n_points, level_t max_depth, level_t trie_depth,
                      preorder_t max_tree_nodes) {
    auto range = (symbol_t) pow(2, max_depth);
    auto *mdtrie = new md_trie<DIMENSION_RANDOM>(max_depth, trie_depth, max_tree_nodes);
    auto *leaf_point = new data_point<DIMENSION_RANDOM>();

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {
        for (dimension_t i = 0; i < DIMENSION_RANDOM; i++) {
            leaf_point->set_coordinate(i, (point_t) rand() % range);
        }
        // 
        mdtrie->insert_trie(leaf_point, max_depth);
        if (!mdtrie->check(leaf_point, max_depth)) {
            raise(SIGINT);
            return false;
        }
    }
    return true;
}

bool test_contiguous_data(n_leaves_t n_points, level_t max_depth, level_t trie_depth,
                          preorder_t max_tree_node) {
    auto range = (symbol_t) pow(2, max_depth);
    auto *mdtrie = new md_trie<DIMENSION_CONTIGUOUS>(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point<DIMENSION_CONTIGUOUS>();

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {

        auto first_half_value = (symbol_t) rand() % range;
        for (dimension_t i = 0; i < DIMENSION_CONTIGUOUS / 2; i++) {
            leaf_point->set_coordinate(i, first_half_value);
        }
        auto second_half_value = (symbol_t) rand() % range;
        for (dimension_t i = DIMENSION_CONTIGUOUS / 2; i < DIMENSION_CONTIGUOUS; i++) {
            leaf_point->set_coordinate(i, second_half_value);
        }
        // if (itr == 1061){
        //     raise(SIGINT);
        // }
        mdtrie->insert_trie(leaf_point, max_depth);
        if (!mdtrie->check(leaf_point, max_depth)) {
            raise(SIGINT);
            return false;
        }
    }
    return true;
}

bool test_nonexistent_data(n_leaves_t n_points, level_t max_depth, level_t trie_depth,
                           preorder_t max_tree_node) {
    auto range = (symbol_t) pow(2, max_depth);
    auto *mdtrie = new md_trie<DIMENSION_NONEXISTENT>(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point<DIMENSION_NONEXISTENT>();

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {
        for (dimension_t i = 0; i < DIMENSION_NONEXISTENT; i++) {
            leaf_point->set_coordinate(i, rand() % (range / 2));
        }
        mdtrie->insert_trie(leaf_point, max_depth);
    }
    for (n_leaves_t itr = 1; itr <= n_points; itr++) {
        for (dimension_t i = 0; i < DIMENSION_NONEXISTENT; i++) {
            leaf_point->set_coordinate(i, rand() % (range / 2) + range / 2);
        }
        if (mdtrie->check(leaf_point, max_depth)) {
            return false;
        }
    }
    return true;
}



//  n_points, dimensions, max_depth, trie_depth, max_tree_node
TEST_CASE("Check Random Data Insertion", "[trie]") {
    srand(static_cast<unsigned int>(time(0)));
    bool result = test_random_data(100000, 20, 10, 1024);
    REQUIRE(result);
}

TEST_CASE("Check Nonexistent Data", "[trie]") {
    srand(static_cast<unsigned int>(time(0)));
    REQUIRE(test_nonexistent_data(10000, 10, 3, 128));
}

TEST_CASE("Check Contiguous Data", "[trie]") {
    srand(static_cast<unsigned int>(time(0)));
    REQUIRE(test_contiguous_data(10000, 10, 3, 128));
}

