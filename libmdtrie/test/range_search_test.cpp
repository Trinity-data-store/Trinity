#include "catch.hpp"
#include "trie.h"

const int DIMENSION_EXACT = 3;
const symbol_t NUM_BRANCHES_EXACT = pow(2, DIMENSION_EXACT);
const int DIMENSION_RANGE = 8;
const symbol_t NUM_BRANCHES_RANGE = pow(2, DIMENSION_RANGE);

bool test_range_search(n_leaves_t n_points, level_t max_depth, level_t trie_depth,
                       preorder_t max_tree_nodes, uint32_t n_itr,
                       n_leaves_t n_checked_points) {

    /**
        First insert random points into the mdtrie
        Range query random search range
        and check if random points in that search range either is 
        found correctly or not 
    */   

    create_level_to_num_children(std::vector<level_t>(DIMENSION_RANGE, max_depth), std::vector<level_t>(DIMENSION_RANGE, 0), max_depth);

    auto range = (symbol_t) pow(2, max_depth);
    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_nodes);
    auto *leaf_point = new data_point();
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
        mdtrie->insert_trie(leaf_point, itr - 1);
    }
    auto *start_range = new data_point();
    auto *end_range = new data_point();
    auto *found_points = new point_array();
    for (uint32_t itr = 1; itr <= n_itr; itr++) {
        for (dimension_t i = 0; i < DIMENSION_RANGE; i++) {
            start_range->set_coordinate(i, min[i] + rand() % (max[i] - min[i] + 1));
            end_range->set_coordinate(i, start_range->get_coordinate(i) + rand() % (max[i] - start_range->get_coordinate(i) + 1));
        }

        // if (found_points->size() != 0)
        //     raise(SIGINT);
            
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);

        for (n_leaves_t i = 0; i < found_points->size(); i++) {
            data_point *leaf = found_points->at(i);
            if (!mdtrie->check(leaf)) {
                return false;
            }
        }

        n_leaves_t checked_points = 0;
        while (checked_points <= n_checked_points) {

            auto *leaf_check = new data_point();
            for (dimension_t i = 0; i < DIMENSION_RANGE; i++) {
                point_t coordinate = start_range->get_coordinate(i) +
                                     rand() % (end_range->get_coordinate(i) - start_range->get_coordinate(i) + 1);
                leaf_check->set_coordinate(i, coordinate);
            }
            bool found = false;
            for (n_leaves_t q = 0; q < found_points->size(); q++) {
                data_point *found_leaf = found_points->at(q);
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
                if (mdtrie->check(leaf_check)){
                    return false;
                }
            } else {
                if (!mdtrie->check(leaf_check)){
                    return false;
                }
            }
        }

        // if (found_points->size() < 0)
        //     raise(SIGINT);

        found_points->reset();
    }
    return true;
}

bool test_range_search_exact(n_leaves_t n_points, level_t max_depth, level_t trie_depth,
                       preorder_t max_tree_nodes, uint32_t n_itr = 50) {

    /**
        First insert random points into the mdtrie
        Range query random search range
        Iterate over the search range
        Check if all points in that search range is found
        All points out of that search range is not found
    */   

    create_level_to_num_children(std::vector<level_t>(DIMENSION_EXACT, max_depth), std::vector<level_t>(DIMENSION_EXACT, 0), max_depth);

    auto range = (symbol_t) pow(2, max_depth);
    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_nodes);
    auto *leaf_point = new data_point();
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
        mdtrie->insert_trie(leaf_point, itr - 1);
    }
    auto *start_range = new data_point();
    auto *end_range = new data_point();
    auto *found_points = new point_array();

    for (uint32_t itr = 1; itr <= n_itr; itr++) {
        for (dimension_t i = 0; i < DIMENSION_EXACT; i++) {
            start_range->set_coordinate(i, min[i] + rand() % (max[i] - min[i] + 1));
            end_range->set_coordinate(i, start_range->get_coordinate(i) + rand() % (max[i] - start_range->get_coordinate(i) + 1));
        }
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);

        for (point_t i = start_range->get_coordinate(0); i <= end_range->get_coordinate(0); i++){
            for (point_t j = start_range->get_coordinate(1); j <= end_range->get_coordinate(1); j++){
                for (point_t k = start_range->get_coordinate(2); k <= end_range->get_coordinate(2); k++){
                    auto *leaf_check = new data_point();
                    leaf_check->set_coordinate(0, i);
                    leaf_check->set_coordinate(1, j);
                    leaf_check->set_coordinate(2, k);

                    bool found = false;
                    for (n_leaves_t q = 0; q < found_points->size(); q++) {
                        data_point *found_leaf = found_points->at(q);
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
                        if (mdtrie->check(leaf_check)){
                            return false;
                        }
                    } else {
                        if (!mdtrie->check(leaf_check)){
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

TEST_CASE("Test Range Search", "[trie]") {
    srand(static_cast<unsigned int>(time(0)));
    REQUIRE(test_range_search(50000, 10, 3, 128, 5, 100));
}


TEST_CASE("Test Exact Range Search", "[trie]") {
    srand(static_cast<unsigned int>(time(0)));
    REQUIRE(test_range_search_exact(1000, 10, 3, 128, 3));
}
