#include "catch.hpp"
#include "trie.h"

bool test_random_data(n_leaves_t n_points, dimension_t dimensions, level_t max_depth, level_t trie_depth,
                      preorder_t max_tree_nodes) {
    auto range = (symbol_t) pow(2, max_depth);
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_nodes);
    auto *leaf_point = new data_point(dimensions);

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {
        for (dimension_t i = 0; i < dimensions; i++) {
            leaf_point->coordinates[i] = (point_t) rand() % range;
        }
        mdtrie->insert_trie(leaf_point, max_depth);
        if (!mdtrie->check(leaf_point, max_depth)) {
            return false;
        }
    }
    return true;
}

bool test_contiguous_data(n_leaves_t n_points, dimension_t dimensions, level_t max_depth, level_t trie_depth,
                          preorder_t max_tree_node) {
    auto range = (symbol_t) pow(2, max_depth);
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point(dimensions);

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {

        auto first_half_value = (symbol_t) rand() % range;
        for (dimension_t i = 0; i < dimensions / 2; i++) {
            leaf_point->coordinates[i] = first_half_value;
        }
        auto second_half_value = (symbol_t) rand() % range;
        for (dimension_t i = dimensions / 2; i < dimensions; i++) {
            leaf_point->coordinates[i] = second_half_value;
        }
        mdtrie->insert_trie(leaf_point, max_depth);
        if (!mdtrie->check(leaf_point, max_depth)) {
            return false;
        }
    }
    return true;
}

bool test_nonexistent_data(n_leaves_t n_points, dimension_t dimensions, level_t max_depth, level_t trie_depth,
                           preorder_t max_tree_node) {
    auto range = (symbol_t) pow(2, max_depth);
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point(dimensions);

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {
        for (dimension_t i = 0; i < dimensions; i++) {
            leaf_point->coordinates[i] = rand() % (range / 2);
        }
        mdtrie->insert_trie(leaf_point, max_depth);
    }
    for (n_leaves_t itr = 1; itr <= n_points; itr++) {
        for (dimension_t i = 0; i < dimensions; i++) {
            leaf_point->coordinates[i] = rand() % (range / 2) + range / 2;
        }
        if (mdtrie->check(leaf_point, max_depth)) {
            return false;
        }
    }
    return true;
}

bool test_range_search(n_leaves_t n_points, dimension_t dimensions, level_t max_depth, level_t trie_depth,
                       preorder_t max_tree_nodes, uint32_t n_itr = 50, bool check = true,
                       n_leaves_t n_checked_points = 100000) {
    auto range = (symbol_t) pow(2, max_depth);
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_nodes);
    auto *leaf_point = new data_point(dimensions);
    auto max = (uint64_t *) malloc(dimensions * sizeof(uint64_t));
    auto min = (uint64_t *) malloc(dimensions * sizeof(uint64_t));

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {

        for (dimension_t i = 0; i < dimensions; i++) {
            leaf_point->coordinates[i] = rand() % range;
            if (itr == 1) {
                max[i] = leaf_point->coordinates[i];
                min[i] = leaf_point->coordinates[i];
            } else {
                if (leaf_point->coordinates[i] > max[i]) {
                    max[i] = leaf_point->coordinates[i];
                }
                if (leaf_point->coordinates[i] < min[i]) {
                    min[i] = leaf_point->coordinates[i];
                }
            }
        }
        mdtrie->insert_trie(leaf_point, max_depth);
    }
    auto *start_range = new data_point(dimensions);
    auto *end_range = new data_point(dimensions);
    auto *found_points = new point_array();
    uint8_t *representation = (uint8_t *) malloc(sizeof(uint8_t) * dimensions);

    for (uint32_t itr = 1; itr <= n_itr; itr++) {
        for (dimension_t i = 0; i < dimensions; i++) {
            start_range->coordinates[i] = min[i] + rand() % (max[i] - min[i] + 1);
            end_range->coordinates[i] =
                    start_range->coordinates[i] + rand() % (max[i] - start_range->coordinates[i] + 1);
        }

        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points, representation);

        if (!check) {
            found_points->reset();
            continue;
        }
        for (n_leaves_t i = 0; i < found_points->size(); i++) {
            data_point *leaf = found_points->at(i);
            if (!mdtrie->check(leaf, max_depth)) {
                return false;
            }
        }

        n_leaves_t checked_points = 0;
        while (checked_points <= n_checked_points) {
            auto *leaf_check = new data_point(dimensions);
            for (dimension_t i = 0; i < dimensions; i++) {
                point_t coordinate = start_range->coordinates[i] +
                                     rand() % (end_range->coordinates[i] - start_range->coordinates[i] + 1);
                leaf_check->coordinates[i] = coordinate;
            }
            bool found = false;
            for (n_leaves_t q = 0; q < found_points->size(); q++) {
                data_point *found_leaf = found_points->at(q);
                found = true;
                for (dimension_t i = 0; i < dimensions; i++) {
                    if (found_leaf->coordinates[i] != leaf_check->coordinates[i]) {
                        found = false;
                        break;
                    }
                }
                if (found)
                    break;
            }
            if (!found) {
                checked_points++;
                if (mdtrie->check(leaf_check, max_depth))
                    return false;
            } else {
                if (!mdtrie->check(leaf_check, max_depth))
                    return false;
            }
        }
        found_points->reset();
    }
    return true;
}

//  n_points, dimensions, max_depth, trie_depth, max_tree_node
TEST_CASE("Check Random Data Insertion", "[trie]") {
    REQUIRE(test_random_data(100000, 8, 20, 10, 1024));
}

TEST_CASE("Check Nonexistent Data", "[trie]") {
    REQUIRE(test_nonexistent_data(10000, 10, 10, 3, 128));
}

TEST_CASE("Check Contiguous Data", "[trie]") {
    REQUIRE(test_contiguous_data(10000, 10, 10, 3, 128));
}

// int n_points, int dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_nodes, int n_itr, int n_checked_points
TEST_CASE("Test Range Search", "[trie]") {
    srand(static_cast<unsigned int>(time(0)));
    REQUIRE(test_range_search(5000, 8, 10, 3, 128, 50, 1000));
}

