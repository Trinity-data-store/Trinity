#include "catch.hpp"
#include "trie.h"
#include <mutex>
#include <thread>
#include <rand_utils.h>

const int DIMENSION = 2;

level_t max_depth = 32;
level_t trie_depth = 10;
preorder_t max_tree_node = 1024;
n_leaves_t n_points = 500000;
uint8_t max_num_threads = 18;
const uint32_t read_number_count = 10000000;

void test_random_insert(md_trie<DIMENSION> *mdtrie){

    auto *leaf_point = new data_point<DIMENSION>();
    symbol_t range = pow(2, max_depth);
    auto *rand_generator = new utils::rand_utils();

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {

        for (dimension_t i = 0; i < DIMENSION; i++) {
            point_t coordinate = (point_t) rand_generator->rand_uint64(0, range - 1);
            leaf_point->set_coordinate(i, coordinate);
        }
        
        mdtrie->insert_trie(leaf_point, max_depth);
    }
    return;    
}

void test_random_read(md_trie<DIMENSION> *mdtrie){

    auto *leaf_point = new data_point<DIMENSION>();
    symbol_t range = pow(2, max_depth);
    auto *rand_generator = new utils::rand_utils();

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {

        for (dimension_t i = 0; i < DIMENSION; i++) {
            point_t coordinate = (point_t) rand_generator->rand_uint64(0, range - 1);
            leaf_point->set_coordinate(i, coordinate);
        }
        
        mdtrie->check(leaf_point, max_depth);
    }
    return;    
}

TEST_CASE("Test random insert and reads", "[trie]") {

    /**
        Test concurrent reads and writes (5 reads and 5 writes)
        See if there is error or deadlocks
    */

    auto *mdtrie = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);

    unsigned int max_num_threads = std::thread::hardware_concurrency();

    uint8_t num_threads = 10;

    std::thread *threads = new std::thread[num_threads];

    for (uint8_t i = 0; i < num_threads; i++){

        if (i % 2 == 0)
            threads[i] = std::thread(test_random_insert, mdtrie);
        else
            threads[i] = std::thread(test_random_read, mdtrie);
    }

    for (uint8_t i = 0; i < num_threads; i++){
        threads[i].join();
    }
}