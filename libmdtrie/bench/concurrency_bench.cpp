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
#include <rand_utils.h>

const int DIMENSION = 3;
level_t max_depth = 32;
level_t trie_depth = 10;
preorder_t max_tree_node = 1024;
n_leaves_t n_points = 1000000;
const uint8_t max_num_threads = 18;
const uint32_t read_number_count = 10000000;

typedef unsigned long long int TimeStamp;

static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

void test_concurrency(md_trie<DIMENSION> *mdtrie){

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

void test_func(){

    usleep(1000000);
}

void vector_insertion(){

    symbol_t range = pow(2, max_depth);
    auto *rand_generator = new utils::rand_utils();
    std::vector<point_t> myVector;

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {
        
        myVector.push_back((point_t) rand_generator->rand_uint64(0, range - 1));
    }    
}


void test_insert_concurrency(){
    
    auto *mdtrie = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);

    fprintf(stderr, "Dimension: %d\n", DIMENSION);
    srand(static_cast<unsigned int>(time(0)));

    unsigned int n = std::thread::hardware_concurrency();
    // 36 concurrent threads are supported
    // Pick 18 threads since 18 cores per socket
    std::cout << n << " concurrent threads are supported.\n";

    for (uint8_t num_threads = 1; num_threads <= max_num_threads; num_threads ++){
        std::thread *t_array = new std::thread[num_threads];

        TimeStamp start = GetTimestamp();
        for (uint8_t i = 0; i < num_threads; i++){
            
            t_array[i] = std::thread(test_concurrency, mdtrie);
        }

        for (uint8_t i = 0; i < num_threads; i++){
            t_array[i].join();
        }
        TimeStamp diff = GetTimestamp() - start;

        uint64_t total_points = n_points * num_threads;

        fprintf(stderr, "Total time to insert %ld points with %d threads: %lld us\n", total_points, num_threads, diff);
        fprintf(stderr, "Throughput: %f per us\n", (float) total_points / diff);

    }

}
TimeStamp total_read_latency = 0;
uint64_t total_count = 0;

void read_mdtrie_inserted(md_trie<DIMENSION> *mdtrie, uint8_t thread_num, uint8_t total_thread_num){

    auto *leaf_point = new data_point<DIMENSION>();
    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    char *saveptr;
    // auto *rand_generator = new utils::rand_utils();
    
    FILE *fp = fopen("../libmdtrie/bench/data/sample_shuf.txt", "r");
    uint64_t count = 0;

    while ((read = getline(&line, &len, fp)) != -1)
    {

        count ++;

        // if (count == read_number_count){
        //     break;
        // }
        
        if (count % total_thread_num != thread_num){
            continue;
        }

        char *token = strtok_r(line, " ", &saveptr);
        char *ptr;
        
        // Skip the second and third token
        for (uint8_t i = 0; i < 2; i ++){
            token = strtok_r(nullptr, " ", &saveptr);
        }

        for (dimension_t i = 0; i < DIMENSION; i++){
            if (i >= 4){
                leaf_point->set_coordinate(i, leaf_point->get_coordinate(i % 4));
            }
            else {
                token = strtok_r(nullptr, " ", &saveptr);
                leaf_point->set_coordinate(i, strtoul(token, &ptr, 10));
            }
        }
        TimeStamp start = GetTimestamp();
        if (!mdtrie->check(leaf_point, max_depth)){
            fprintf(stderr, "error! not found\n");
        }
        total_read_latency += GetTimestamp() - start;
        total_count ++;
    }
    
    fclose(fp);
    return;
}


void test_read_concurrency(){

    auto *mdtrie = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point<DIMENSION>();

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../libmdtrie/bench/data/sample_shuf.txt", "r");

    
    n_leaves_t n_points = 0;
    n_leaves_t n_lines = 14583357;
    uint64_t max[DIMENSION];
    uint64_t min[DIMENSION];
    tqdm bar1;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        // if (n_points == read_number_count){
        //     break;
        // }
        bar1.progress(n_points, n_lines);
        // Get the first token
        char *token = strtok(line, " ");
        char *ptr;
        // Skip the second and third token
        for (uint8_t i = 0; i < 2; i ++){
            token = strtok(nullptr, " ");
        }
        for (dimension_t i = 0; i < DIMENSION; i++){
            if (i >= 4){
                leaf_point->set_coordinate(i, leaf_point->get_coordinate(i % 4));
            }
            else {
                token = strtok(nullptr, " ");
                leaf_point->set_coordinate(i, strtoul(token, &ptr, 10));
            }
            if (n_points == 0){
                max[i] = leaf_point->get_coordinate(i);
                min[i] = leaf_point->get_coordinate(i);
            }
            else {
                if (leaf_point->get_coordinate(i) > max[i]){
                    max[i] = leaf_point->get_coordinate(i);
                }
                if (leaf_point->get_coordinate(i) < min[i]){
                    min[i] = leaf_point->get_coordinate(i);
                }
            }
        }
        mdtrie->insert_trie(leaf_point, max_depth);
        n_points ++;
    }
    bar1.finish();
    fclose(fp);
    fprintf(stderr, "reading existing points begins\n");


    // *******************************************************
    // uint8_t num_threads = 9;
    for (uint8_t num_threads = 3; num_threads <= max_num_threads; num_threads ++){
        std::thread *t_array = new std::thread[num_threads];

        // num_checked_points = 0;

        TimeStamp start = GetTimestamp();
        for (uint8_t i = 0; i < num_threads; i++){
            
            t_array[i] = std::thread(read_mdtrie_inserted, mdtrie, i, num_threads);
            // t_array[i] = std::thread(read_mdtrie_random, mdtrie, max, min);
        }

        for (uint8_t i = 0; i < num_threads; i++){
            t_array[i].join();
        }
        TimeStamp diff = GetTimestamp() - start;

        uint64_t total_points = n_points * num_threads;
        total_points = n_points;
        // total_points = read_number_count * num_threads;

        fprintf(stderr, "Total time to read %ld points with %d threads: %lld us\n", total_points, num_threads, diff);
        fprintf(stderr, "Throughput: %f per us\n", (float) total_points / diff);
        fprintf(stderr, "Latency per read %f\n", (float)total_read_latency / total_count);
        total_read_latency = 0;
        total_count = 0;
    }

}

const int n_itr = 100;
uint64_t total_found_points = 0;

void range_search_mdtrie(md_trie<DIMENSION> *mdtrie, uint64_t max[], uint64_t min[]){

    auto *start_range = new data_point<DIMENSION>();
    auto *end_range = new data_point<DIMENSION>();
    auto *found_points = new point_array<DIMENSION>();
    int itr = 1;

    TimeStamp lookup_time = 0;
    while (itr <= n_itr){

        for (dimension_t i = 0; i < DIMENSION; i++){
            start_range->set_coordinate(i,  min[i] + rand() % (max[i] - min[i] + 1));
            end_range->set_coordinate(i, start_range->get_coordinate(i) + rand() % (max[i] - start_range->get_coordinate(i) + 1));
            volume *= (end_range->get_coordinate(i) - start_range->get_coordinate(i));
        }
        start = GetTimestamp();
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
        diff = GetTimestamp() - start;

        total_found_points += found_points->size();
        
        found_points->reset();
        itr++;
           
    }
    fclose(fptr);
}

void test_range_search(){

    auto *mdtrie = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point<DIMENSION>();

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../libmdtrie/bench/data/sample_shuf.txt", "r");

    n_leaves_t n_points = 0;
    n_leaves_t n_lines = 14583357;
    uint64_t max[DIMENSION];
    uint64_t min[DIMENSION];
    tqdm bar1;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar1.progress(n_points, n_lines);
        // Get the first token
        char *token = strtok(line, " ");
        char *ptr;
        // Skip the second and third token
        for (uint8_t i = 0; i < 2; i ++){
            token = strtok(nullptr, " ");
        }
        for (dimension_t i = 0; i < DIMENSION; i++){
            if (i >= 4){
                leaf_point->set_coordinate(i, leaf_point->get_coordinate(i % 4));
            }
            else {
                token = strtok(nullptr, " ");
                leaf_point->set_coordinate(i, strtoul(token, &ptr, 10));
            }
            if (n_points == 0){
                max[i] = leaf_point->get_coordinate(i);
                min[i] = leaf_point->get_coordinate(i);
            }
            else {
                if (leaf_point->get_coordinate(i) > max[i]){
                    max[i] = leaf_point->get_coordinate(i);
                }
                if (leaf_point->get_coordinate(i) < min[i]){
                    min[i] = leaf_point->get_coordinate(i);
                }
            }
        }
        mdtrie->insert_trie(leaf_point, max_depth);
        n_points ++;
    }
    bar1.finish();
    fclose(fp);
    fprintf(stderr, "reading existing points begins\n");

    // *******************************************************
    for (uint8_t num_threads = 3; num_threads <= max_num_threads; num_threads ++){
        std::thread *t_array = new std::thread[num_threads];

        TimeStamp start = GetTimestamp();
        for (uint8_t i = 0; i < num_threads; i++){
            
            t_array[i] = std::thread(range_search_mdtrie, mdtrie, max, min);
        }

        for (uint8_t i = 0; i < num_threads; i++){
            t_array[i].join();
        }
        TimeStamp diff = GetTimestamp() - start;

        fprintf(stderr, "Total time to read %ld points with %d threads: %lld us\n", total_points, num_threads, diff);
        fprintf(stderr, "Throughput: %f per us\n", (float) total_found_points / diff);
    }
}


int main() {

    test_range_search();
    // test_insert_concurrency();
    // test_read_concurrency();

}
