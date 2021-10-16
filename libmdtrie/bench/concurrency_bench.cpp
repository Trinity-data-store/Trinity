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
const symbol_t NUM_BRANCHES = pow(2, DIMENSION);

level_t max_depth = 32;
level_t trie_depth = 10;
preorder_t max_tree_node = 1024;
n_leaves_t n_points = 1000000;
uint8_t max_num_threads = 18;
const uint32_t read_number_count = 1000000;


void test_random_insert(md_trie *mdtrie){

    auto *leaf_point = new data_point();
    // TODO: bug, smaller range raise an error
    symbol_t range = pow(2, max_depth);
    auto *rand_generator = new utils::rand_utils();

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {

        for (dimension_t i = 0; i < DIMENSION; i++) {
            point_t coordinate = (point_t) rand_generator->rand_uint64(0, range - 1);
            leaf_point->set_coordinate(i, coordinate);
        }
        
        mdtrie->insert_trie(leaf_point, itr - 1);
    }
    return;    
}

void test_random_read(md_trie *mdtrie){

    auto *leaf_point = new data_point();
    symbol_t range = pow(2, max_depth);
    auto *rand_generator = new utils::rand_utils();

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {

        for (dimension_t i = 0; i < DIMENSION; i++) {
            point_t coordinate = (point_t) rand_generator->rand_uint64(0, range - 1);
            leaf_point->set_coordinate(i, coordinate);
        }
        
        mdtrie->check(leaf_point);
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
    
    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);

    fprintf(stderr, "Dimension: %d\n", DIMENSION);
    srand(static_cast<unsigned int>(time(0)));

    unsigned int n = std::thread::hardware_concurrency();
    // 36 concurrent threads are supported
    // Pick 18 threads since 18 cores per socket
    std::cout << n << " concurrent threads are supported.\n";

    for (uint8_t num_threads = 1; num_threads <= max_num_threads; num_threads ++){
        std::thread *threads = new std::thread[num_threads];

        TimeStamp start = GetTimestamp();
        for (uint8_t i = 0; i < num_threads; i++){
            
            threads[i] = std::thread(test_random_insert, mdtrie);
        }

        for (uint8_t i = 0; i < num_threads; i++){
            threads[i].join();
        }
        TimeStamp diff = GetTimestamp() - start;

        uint64_t total_points = n_points * num_threads;

        fprintf(stderr, "Total time to insert %ld points with %d threads: %lld us\n", total_points, num_threads, diff);
        fprintf(stderr, "Throughput: %f per us\n", (float) total_points / diff);

    }

}

std::vector<data_point *>get_dataset_vector()
{
    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../libmdtrie/bench/data/sample_shuf.txt", "r");

    n_leaves_t n_points = 0;
    n_leaves_t n_lines = 14583357;
    tqdm bar1;
    std::vector<data_point *> vect;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        auto *leaf_point = new data_point();
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
        }
        vect.push_back(leaf_point);
        n_points ++;
    }
    return vect;
}


void insert_mdtrie_from_vector(md_trie *mdtrie, uint8_t thread_num, uint8_t total_thread_num, std::vector<data_point *> vect){

    n_leaves_t n_lines = 14583357;
    for (uint64_t count = thread_num; count < n_lines; count += total_thread_num)
    {
        data_point *leaf_point = vect[count];
        // TimeStamp start = GetTimestamp();
        mdtrie->insert_trie(leaf_point, thread_num);
        
        // total_read_latency += GetTimestamp() - start;
        // total_count ++;
    }
    return;
}

void test_insert_concurrency_real_dataset(){
    
    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);
    std::vector<data_point *> vect = get_dataset_vector();
    fprintf(stderr, "Dimension: %d\n", DIMENSION);
    srand(static_cast<unsigned int>(time(0)));

    unsigned int n = std::thread::hardware_concurrency();
    // 36 concurrent threads are supported
    // Pick 18 threads since 18 cores per socket
    std::cout << n << " concurrent threads are supported.\n";

    for (uint8_t num_threads = 1; num_threads <= max_num_threads; num_threads ++){
        std::thread *threads = new std::thread[num_threads];

        TimeStamp start = GetTimestamp();
        for (uint8_t i = 0; i < num_threads; i++){
            
            threads[i] = std::thread(insert_mdtrie_from_vector, mdtrie, i, num_threads, vect);
        }

        for (uint8_t i = 0; i < num_threads; i++){
            threads[i].join();
        }
        TimeStamp diff = GetTimestamp() - start;

        uint64_t total_points = 14583357 * num_threads;

        fprintf(stderr, "Total time to insert %ld points with %d threads: %lld us\n", total_points, num_threads, diff);
        fprintf(stderr, "Throughput: %f per us\n", (float) total_points / diff);

    }

}


void random_read_mdtrie_inserted_from_vector(md_trie *mdtrie, uint8_t total_thread_num, std::vector<data_point *> *vect){

    n_leaves_t n_lines = 14583357;
    uint64_t total_iter = n_lines / total_thread_num;
    uint64_t count = 0;
    auto *rand_generator = new utils::rand_utils();
    while (count < total_iter){
        count ++;

        data_point *leaf_point = (*vect)[rand_generator->rand_uint64(0, n_lines - 1)];
        // TimeStamp start = GetTimestamp();
        if (!mdtrie->check(leaf_point)){
            fprintf(stderr, "error! not found\n");
        }
        // total_read_latency += GetTimestamp() - start;
        // total_count ++;
    }
    return;
}


void read_mdtrie_inserted_from_vector(md_trie *mdtrie, uint8_t thread_num, uint8_t total_thread_num, std::vector<data_point *> *vect){

    n_leaves_t n_lines = 14583357;
    for (uint64_t count = thread_num; count < n_lines; count += total_thread_num){

        data_point *leaf_point = (*vect)[count];
        // TimeStamp start = GetTimestamp();
        if (!mdtrie->check(leaf_point)){
            fprintf(stderr, "error! not found\n");
        }
        // total_read_latency += GetTimestamp() - start;
        // total_count ++;
    }
    return;
}

void test_read_concurrency(){

    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);
    
    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../libmdtrie/bench/data/sample_shuf.txt", "r");

    n_leaves_t n_points = 0;
    n_leaves_t n_lines = 14583357;
    tqdm bar1;
    std::vector<data_point *> vect;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        auto *leaf_point = new data_point();
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
        }
        vect.push_back(leaf_point);
        mdtrie->insert_trie(leaf_point, n_points);
        n_points ++;
    }
    if (n_points != n_lines){
        fprintf(stderr, "correct num lines: %ld\n", n_points);
    }
    bar1.finish();
    fclose(fp);
    fprintf(stderr, "reading existing points begins\n");

    // *******************************************************

    std::cerr << "Maximum number of threads: " << std::thread::hardware_concurrency() << "\n";    

    for (uint8_t num_threads = 1; num_threads <= max_num_threads; num_threads ++){

        TimeStamp start = GetTimestamp();

        std::vector<std::thread> threads(num_threads);

        for (uint8_t i = 0; i < num_threads; i++)
        {    
            // TODO: pass vector by reference
            threads[i] = std::thread(random_read_mdtrie_inserted_from_vector, mdtrie, num_threads, &vect);
            // threads[i] = std::thread(read_mdtrie_inserted_from_vector, mdtrie, i, num_threads, &vect);

            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(i, &cpuset);
            sched_setaffinity(threads[i].native_handle(), sizeof(cpu_set_t), &cpuset);
        }

        for (uint8_t i = 0; i < num_threads; i++){
            threads[i].join();
        }

        TimeStamp diff = GetTimestamp() - start;
        
        uint64_t total_points = n_points * num_threads;
        total_points = n_points;

        fprintf(stderr, "Total time to read %ld points with %d threads: %lld us\n", total_points, num_threads, diff);
        fprintf(stderr, "Throughput: %f per us\n", (float) total_points / diff);
        // fprintf(stderr, "Latency per read %f\n", (float)total_read_latency / total_count);
        // total_read_latency = 0;
        // total_count = 0;

        std::cerr << "\n";
    }

}

const int n_itr = 100;
uint64_t total_found_points = 0;
TimeStamp total_range_search_latency = 0;
void range_search_mdtrie(md_trie *mdtrie, uint64_t max[], uint64_t min[]){

    auto *start_range = new data_point();
    auto *end_range = new data_point();
    auto *found_points = new point_array();
    int itr = 1;

    while (itr <= n_itr){

        for (dimension_t i = 0; i < DIMENSION; i++){
            start_range->set_coordinate(i,  min[i] + rand() % (max[i] - min[i] + 1));
            end_range->set_coordinate(i, start_range->get_coordinate(i) + rand() % (max[i] - start_range->get_coordinate(i) + 1));
        }
        TimeStamp start = GetTimestamp();
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
        total_range_search_latency += GetTimestamp() - start;

        total_found_points += found_points->size();
        
        found_points->reset();
        itr++;
    }
}

void test_range_search(){

    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point();

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
        mdtrie->insert_trie(leaf_point, n_points);
        n_points ++;
    }
    bar1.finish();
    fclose(fp);
    fprintf(stderr, "reading existing points begins\n");

    // *******************************************************
    for (uint8_t num_threads = 1; num_threads <= max_num_threads; num_threads ++){
        std::thread *threads = new std::thread[num_threads];

        TimeStamp start = GetTimestamp();
        for (uint8_t i = 0; i < num_threads; i++){
            
            threads[i] = std::thread(range_search_mdtrie, mdtrie, max, min);
        }

        for (uint8_t i = 0; i < num_threads; i++){
            threads[i].join();
        }
        TimeStamp diff = GetTimestamp() - start;

        fprintf(stderr, "Total time to read %ld points with %d threads: %lld us\n", total_found_points, num_threads, diff);
        fprintf(stderr, "Throughput: %f per us\n", (float) total_found_points / diff);
        fprintf(stderr, "Latency per read %f\n", (float)total_range_search_latency / total_found_points);
        total_range_search_latency = 0;
    }
}

void random_range_search_mdtrie(md_trie *mdtrie){

    auto *start_range = new data_point();
    auto *end_range = new data_point();
    auto *found_points = new point_array();
    int itr = 1;
    symbol_t range = pow(2, max_depth);
    auto *rand_generator = new utils::rand_utils();
    
    while (itr <= n_itr){

        for (dimension_t i = 0; i < DIMENSION; i++){
            point_t start = (point_t) rand_generator->rand_uint64(0, range - 2);
            point_t end = (point_t) rand_generator->rand_uint64(start + 1, range - 1);
            start_range->set_coordinate(i,  start);
            end_range->set_coordinate(i, end);
        }
        TimeStamp start = GetTimestamp();
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
        total_range_search_latency += GetTimestamp() - start;

        total_found_points += found_points->size();
        
        found_points->reset();
        itr++;
    }
}

void test_read_insert_random(){

    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);

    fprintf(stderr, "Dimension: %d\n", DIMENSION);
    srand(static_cast<unsigned int>(time(0)));

    unsigned int n = std::thread::hardware_concurrency();
    // 36 concurrent threads are supported
    // Pick 18 threads since 18 cores per socket
    std::cout << n << " concurrent threads are supported.\n";

    for (uint8_t num_threads = 1; num_threads <= max_num_threads; num_threads ++){
        std::thread *threads = new std::thread[num_threads];

        TimeStamp start = GetTimestamp();
        for (uint8_t i = 0; i < num_threads; i++){

            // if (i % 3 == 0)
            //     threads[i] = std::thread(test_random_insert, mdtrie);
            // else if (i % 3 == 1)
            //     threads[i] = std::thread(random_range_search_mdtrie, mdtrie);
            // else
            //     threads[i] = std::thread(test_random_read, mdtrie);


            if (i % 2 == 0)
                threads[i] = std::thread(test_random_insert, mdtrie);
            else
                threads[i] = std::thread(test_random_read, mdtrie);
        }

        for (uint8_t i = 0; i < num_threads; i++){
            threads[i].join();
        }
        TimeStamp diff = GetTimestamp() - start;

        uint64_t total_points = n_points * num_threads;

        fprintf(stderr, "Total time to insert/read %ld points with %d threads: %lld us\n", total_points, num_threads, diff);
        fprintf(stderr, "Throughput: %f per us\n", (float) total_points / diff);

    }    



}
int main() {

    // test_insert_concurrency();
    test_read_insert_random();
    // test_range_search();
    // test_insert_concurrency_real_dataset();
    // test_read_concurrency();

}
