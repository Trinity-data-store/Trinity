#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <tqdm.h>
#include <vector>
#include <math.h> 
#include <iostream>
#include <thread>
#include <mutex>

const int DIMENSION = 3;
level_t max_depth = 32;
level_t trie_depth = 10;
preorder_t max_tree_node = 1024;
n_leaves_t n_points = 100000;
const uint8_t max_num_threads = 70;

typedef unsigned long long int TimeStamp;

static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

void test_concurrency(md_trie<DIMENSION> *mdtrie){

    auto *leaf_point = new data_point<DIMENSION>();
    symbol_t range = pow(2, max_depth);

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {
        for (dimension_t i = 0; i < DIMENSION; i++) {
            leaf_point->set_coordinate(i, (point_t) rand() % range);
        }

        mdtrie->insert_trie(leaf_point, max_depth);
    }
    return;    
}

int main() {

    auto *mdtrie = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);

    fprintf(stderr, "Dimension: %d\n", DIMENSION);
    
    for (uint8_t num_threads = 10; num_threads < max_num_threads; num_threads += 10){
        std::thread *t_array = new std::thread[num_threads];
        for (uint8_t i = 0; i < num_threads; i++){
            t_array[i] = std::thread(test_concurrency, mdtrie);
        }

        TimeStamp start = GetTimestamp();
        for (uint8_t i = 0; i < num_threads; i++){
            srand(static_cast<unsigned int>(time(0) + i));
            t_array[i].join();
        }
        TimeStamp diff = GetTimestamp() - start;

        uint64_t msec = diff * 1000 / CLOCKS_PER_SEC;
        uint64_t total_points = n_points * num_threads;
        
        fprintf(stderr, "Total time to insert %ld points with %d threads: %ld us\n", total_points, num_threads, msec*1000);
        fprintf(stderr, "Throughput: %f\n", (float) total_points / msec*1000);
        // fprintf(stderr, "Average Time per point: %f us. Total Latency / Total number of points\n\n", (float)msec*1000 / total_points);
    }
}
