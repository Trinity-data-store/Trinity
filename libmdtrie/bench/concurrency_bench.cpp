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
n_leaves_t n_points = 10000000;
const uint8_t max_num_threads = 36;

typedef unsigned long long int TimeStamp;

static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp) now.tv_sec * 10000000;
}

// void test_concurrency(md_trie<DIMENSION> *mdtrie){
void test_concurrency(int i){

    // fprintf(stderr, "start: thread %d\n", i);
    auto *mdtrie_1 = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point<DIMENSION>();
    symbol_t range = pow(2, max_depth);

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {


        for (dimension_t i = 0; i < DIMENSION; i++) {
            point_t coordinate = (point_t) rand() % range;
            leaf_point->set_coordinate(i, coordinate);
        }
        
        // if (itr % 3 == 0){
        //     fprintf(stderr, "thread %d, start itr %ld\n", i, itr);
        // }

        mdtrie_1->insert_trie(leaf_point, max_depth);

        // if (itr % 3 == 0){
        //     fprintf(stderr, "thread %d, end itr %ld\n", i, itr);
        // }

    }
    // fprintf(stderr, "end: thread %d\n", i);
    return;    
}

void test_func(){

    usleep(1000000);
}

void vector_insertion(){

    symbol_t range = pow(2, max_depth);

    std::vector<point_t> myVector;
    for (n_leaves_t itr = 1; itr <= n_points; itr++) {
        
        // point_t coordinate = (point_t) rand() + 1;
        myVector.push_back(itr + 1);
    }    


}

int main() {

    
    // auto *mdtrie = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);

    fprintf(stderr, "Dimension: %d\n", DIMENSION);
    srand(static_cast<unsigned int>(time(0)));

    unsigned int n = std::thread::hardware_concurrency();
    // 36 concurrent threads are supported
    std::cout << n << " concurrent threads are supported.\n";

    for (uint8_t num_threads = 1; num_threads <= max_num_threads; num_threads ++){
        std::thread *t_array = new std::thread[num_threads];

        TimeStamp start = GetTimestamp();
        for (uint8_t i = 0; i < num_threads; i++){
            
            // Works: 
            // t_array[i] = std::thread(test_func);

            t_array[i] = std::thread(vector_insertion);

            // Doesn't work as expected; throughput stays the same
            // t_array[i] = std::thread(test_concurrency, i);

        }

        for (uint8_t i = 0; i < num_threads; i++){
            t_array[i].join();
        }
        TimeStamp diff = GetTimestamp() - start;

        uint64_t msec = diff * 1000 / CLOCKS_PER_SEC;
        uint64_t total_points = n_points * num_threads;
        
        fprintf(stderr, "Total time to insert %ld points with %d threads: %ld us\n", total_points, num_threads, msec*1000);
        fprintf(stderr, "Throughput: %f\n", (float) total_points / msec*1000);

    }
}
