#include "catch.hpp"
#include "trie.h"

const int DIMENSION_EXACT = 3;
const int DIMENSION_RANGE = 8;

bool test_range_search(n_leaves_t n_points, level_t max_depth, level_t trie_depth,
                       preorder_t max_tree_nodes, uint32_t n_itr = 50, bool check = true,
                       n_leaves_t n_checked_points = 100000) {
    auto range = (symbol_t) pow(2, max_depth);
    auto *mdtrie = new md_trie<DIMENSION_RANGE>(max_depth, trie_depth, max_tree_nodes);
    auto *leaf_point = new data_point<DIMENSION_RANGE>();
    uint64_t max[DIMENSION_RANGE];
    uint64_t min[DIMENSION_RANGE];

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {

        for (dimension_t i = 0; i < DIMENSION_RANGE; i++) {
            leaf_point->set_coordinate(i, rand() % range);
            if (itr == 1) {
                max[i] = leaf_point->get_coordinate(i);
                min[i] = leaf_point->get_coordinate(i);
            } else {
                if (leaf_point->get_coordinate(i) > max[i]) {
                    max[i] = leaf_point->get_coordinate(i);
                }
                if (leaf_point->get_coordinate(i) < min[i]) {
                    min[i] = leaf_point->get_coordinate(i);
                }
            }
        }
        mdtrie->insert_trie(leaf_point, max_depth);
    }
    auto *start_range = new data_point<DIMENSION_RANGE>();
    auto *end_range = new data_point<DIMENSION_RANGE>();
    auto *found_points = new point_array<DIMENSION_RANGE>();
    for (uint32_t itr = 1; itr <= n_itr; itr++) {
        for (dimension_t i = 0; i < DIMENSION_RANGE; i++) {
            start_range->set_coordinate(i, min[i] + rand() % (max[i] - min[i] + 1));
            end_range->set_coordinate(i, start_range->get_coordinate(i) + rand() % (max[i] - start_range->get_coordinate(i) + 1));
        }
    
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);

        if (!check) {
            found_points->reset();
            continue;
        }
        for (n_leaves_t i = 0; i < found_points->size(); i++) {
            data_point<DIMENSION_RANGE> *leaf = found_points->at(i);
            if (!mdtrie->check(leaf, max_depth)) {
                // raise(SIGINT);
                return false;
            }
        }

        n_leaves_t checked_points = 0;
        while (checked_points <= n_checked_points) {
            auto *leaf_check = new data_point<DIMENSION_RANGE>();
            for (dimension_t i = 0; i < DIMENSION_RANGE; i++) {
                point_t coordinate = start_range->get_coordinate(i) +
                                     rand() % (end_range->get_coordinate(i) - start_range->get_coordinate(i) + 1);
                leaf_check->set_coordinate(i, coordinate);
            }
            bool found = false;
            for (n_leaves_t q = 0; q < found_points->size(); q++) {
                data_point<DIMENSION_RANGE> *found_leaf = found_points->at(q);
                found = true;
                for (dimension_t i = 0; i < DIMENSION_RANGE; i++) {
                    if (found_leaf->get_coordinate(i) != leaf_check->get_coordinate(i)) {
                        found = false;
                        break;
                    }
                }
                if (found)
                    break;
            }
            if (!found) {
                checked_points++;
                if (mdtrie->check(leaf_check, max_depth)){
                    // raise(SIGINT);
                    return false;
                }
            } else {
                if (!mdtrie->check(leaf_check, max_depth)){
                    // raise(SIGINT);
                    return false;
                }
            }
        }
        found_points->reset();
    }
    // raise(SIGINT);
    return true;
}

bool test_range_search_exact(n_leaves_t n_points, level_t max_depth, level_t trie_depth,
                       preorder_t max_tree_nodes, uint32_t n_itr = 50) {
    auto range = (symbol_t) pow(2, max_depth);
    auto *mdtrie = new md_trie<DIMENSION_EXACT>(max_depth, trie_depth, max_tree_nodes);
    auto *leaf_point = new data_point<DIMENSION_EXACT>();
    uint64_t max[DIMENSION_EXACT];
    uint64_t min[DIMENSION_EXACT];

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {

        for (dimension_t i = 0; i < DIMENSION_EXACT; i++) {
            leaf_point->set_coordinate(i, rand() % range);
            if (itr == 1) {
                max[i] = leaf_point->get_coordinate(i);
                min[i] = leaf_point->get_coordinate(i);
            } else {
                if (leaf_point->get_coordinate(i) > max[i]) {
                    max[i] = leaf_point->get_coordinate(i);
                }
                if (leaf_point->get_coordinate(i) < min[i]) {
                    min[i] = leaf_point->get_coordinate(i);
                }
            }
        }
        mdtrie->insert_trie(leaf_point, max_depth);
    }
    auto *start_range = new data_point<DIMENSION_EXACT>();
    auto *end_range = new data_point<DIMENSION_EXACT>();
    auto *found_points = new point_array<DIMENSION_EXACT>();

    // raise(SIGINT);
    for (uint32_t itr = 1; itr <= n_itr; itr++) {
        for (dimension_t i = 0; i < DIMENSION_EXACT; i++) {
            start_range->set_coordinate(i, min[i] + rand() % (max[i] - min[i] + 1));
            end_range->set_coordinate(i, start_range->get_coordinate(i) + rand() % (max[i] - start_range->get_coordinate(i) + 1));
        }
        // raise(SIGINT);
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);

        for (point_t i = start_range->get_coordinate(0); i <= end_range->get_coordinate(0); i++){
            for (point_t j = start_range->get_coordinate(1); j <= end_range->get_coordinate(1); j++){
                for (point_t k = start_range->get_coordinate(2); k <= end_range->get_coordinate(2); k++){
                    auto *leaf_check = new data_point<DIMENSION_EXACT>();
                    leaf_check->set_coordinate(0, i);
                    leaf_check->set_coordinate(1, j);
                    leaf_check->set_coordinate(2, k);

                    bool found = false;
                    for (n_leaves_t q = 0; q < found_points->size(); q++) {
                        data_point<DIMENSION_EXACT> *found_leaf = found_points->at(q);
                        found = true;
                        for (dimension_t i = 0; i < DIMENSION_EXACT; i++) {
                            if (found_leaf->get_coordinate(i) != leaf_check->get_coordinate(i)) {
                                found = false;
                                break;
                            }
                        }
                        if (found)
                            break;
                    }
                    if (!found) {
                        if (mdtrie->check(leaf_check, max_depth)){
                            // raise(SIGINT);
                            return false;
                        }
                    } else {
                        if (!mdtrie->check(leaf_check, max_depth)){
                            // raise(SIGINT);
                            return false;
                        }
                    }
                }
            }
        }
        found_points->reset();
    }
    return true;
}


// n_leaves_t n_points, dimension_t dimensions, level_t max_depth, level_t trie_depth, preorder_t max_tree_nodes, uint32_t n_itr = 50
TEST_CASE("Test Exact Range Search", "[trie]") {
    srand(static_cast<unsigned int>(time(0)));
    REQUIRE(test_range_search_exact(2000, 10, 3, 128, 20));
    // raise(SIGINT);
}

// int n_points, int dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_nodes, int n_itr, int n_checked_points
TEST_CASE("Test Range Search", "[trie]") {
    srand(static_cast<unsigned int>(time(0)));
    REQUIRE(test_range_search(5000, 10, 3, 128, 50, 1000));
    // raise(SIGINT);
}