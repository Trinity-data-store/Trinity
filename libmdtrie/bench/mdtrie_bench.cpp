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
#include <fstream>

std::mutex mutex_out;

FILE *fptr;
char file_path[] = "benchmark_output_vector.txt";
char file_path_csv[] = "cdf_8.txt";


const int DIMENSION = 6;
const symbol_t NUM_BRANCHES = pow(2, DIMENSION);


void test_random_data(n_leaves_t n_points, level_t max_depth, level_t trie_depth, preorder_t max_tree_node)
{
    auto range = (symbol_t)pow(2, max_depth);
    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);

    auto *leaf_point = new data_point();

    TimeStamp t0, t1;
    t0 = GetTimestamp();
    for (n_leaves_t itr = 1; itr <= n_points; itr ++){

        for (dimension_t i = 0; i < DIMENSION; i++){
            leaf_point->set_coordinate(i, rand() % range);
        }
        mdtrie->insert_trie(leaf_point, max_depth, itr - 1);
        if (!mdtrie->check(leaf_point, max_depth)){
            fprintf(stderr, "Error insertion!\n");
        }
    }

    t1 = GetTimestamp();
    fprintf(stderr, "Time to insert %ld points with %d dimensions: %llu microseconds\n", n_points, DIMENSION, (t1 - t0));
    fprintf(stderr, "Average time to insert one point: %f microseconds\n", (float) (t1 - t0) / n_points);
    // fprintf(stderr, "Total bimap insertion time: %lld, total bimap count: %ld\n", total_bimap, bimap_insertion_count);
}

void test_real_data(level_t max_depth, level_t trie_depth, preorder_t max_tree_node){

    // fptr = fopen(file_path, "a");
    // fprintf(fptr, "dimensions: %d, trie_depth: %ld, max_tree_node: %ld\n", DIMENSION, trie_depth, max_tree_node);
    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point();

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../libmdtrie/bench/data/sample_shuf.txt", "r");

    // If the file cannot be open
    if (fp == nullptr)
    {
        fprintf(stderr, "file not found\n");
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            printf("Current working dir: %s\n", cwd);
        } else {
            perror("getcwd() error");
        }
        exit(EXIT_FAILURE);
    }

    TimeStamp start, diff;
    
    n_leaves_t n_points = 0;

    n_leaves_t n_lines = 14583357;

    diff = 0;
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
        }
        start = GetTimestamp();
        mdtrie->insert_trie(leaf_point, max_depth, n_points);
        diff += GetTimestamp() - start;
        n_points ++;
    }
    bar1.finish();
    fprintf(stderr, "Average time to insert one point: %f microseconds per insertion\n", (float) diff / n_points);
    fprintf(stderr, "Size of data structure that contains %ld points: %ld bytes\n", n_lines, mdtrie->size());

    // Query n_lines random points
    n_points = 0;
    diff = 0;
    n_leaves_t n_nonexistent = 0;
    tqdm bar2;
    auto range = (symbol_t)pow(2, max_depth);
    while (n_points < n_lines)
    {
        bar2.progress(n_points, n_lines);
        // Get the first token
        for (dimension_t i = 0; i < DIMENSION; i++){
            leaf_point->set_coordinate(i, rand() % range);
        }
        start = GetTimestamp();
        if(!mdtrie->check(leaf_point, max_depth)){
            diff += GetTimestamp() - start;
            n_nonexistent += 1;
        }
        
        n_points ++;
    }
    bar2.finish();
    fprintf(stderr, "Average time to query nonexistent points: %f microseconds per query\n", (float)diff / n_nonexistent); 

    // Time to query the inserted points
    rewind(fp);
    n_points = 0;
    diff = 0;
    tqdm bar3;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar3.progress(n_points, n_lines);
        // Get the first token
        char *token = strtok(line, " ");
        char *ptr;
        // Skip the second and third token
        for (uint8_t i = 0; i < 2; i ++){
            token = strtok(NULL, " ");
        }
        for (dimension_t i = 0; i < DIMENSION; i++){
            token = strtok(NULL, " ");
            leaf_point->set_coordinate(i, strtoul(token, &ptr, 10));
        }
        start = GetTimestamp();
        if (!mdtrie->check(leaf_point, max_depth)){
            raise(SIGINT);
            mdtrie->check(leaf_point, max_depth);
            fprintf(stderr, "error!\n");
        }
        diff += GetTimestamp() - start;
        n_points ++;
    }
    bar3.finish();
    fprintf(stderr, "Average time to query points that are in the trie: %f microseconds per query\n", (float)diff / n_points);
}


void test_insert_data(level_t max_depth, level_t trie_depth, preorder_t max_tree_node){

    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point();

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../libmdtrie/bench/data/sample_shuf.txt", "r");

    // If the file cannot be open
    if (fp == nullptr)
    {
        fprintf(stderr, "file not found\n");
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            printf("Current working dir: %s\n", cwd);
        } else {
            perror("getcwd() error");
        }
        exit(EXIT_FAILURE);
    }

    TimeStamp start, diff;
    
    n_leaves_t n_points = 0;
    n_leaves_t n_lines = 14583357;
    diff = 0;
    tqdm bar;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar.progress(n_points, n_lines);
        // Get the first token
        char *token = strtok(line, " ");
        char *ptr;
        // Skip the second and third token
        for (uint8_t i = 0; i < 2; i ++){
            strtok(nullptr, " ");
        }
        for (dimension_t i = 0; i < DIMENSION; i++){
            // raise(SIGINT);
            if (i >= 4){
                leaf_point->set_coordinate(i, leaf_point->get_coordinate(i % 4));
            }
            else {
                token = strtok(nullptr, " ");
                leaf_point->set_coordinate(i, strtoul(token, &ptr, 10));
            }
        }
        start = GetTimestamp();
        mdtrie->insert_trie(leaf_point, max_depth, n_points);
        diff += GetTimestamp() - start;
        n_points ++;
    }
    bar.finish();
    fprintf(stderr, "Dimension: %d\n", DIMENSION);
    fprintf(stderr, "Average time to insert one point: %f microseconds\n", (float)diff / n_points);
    std::cerr << "Storage: " << mdtrie->size() << " bytes\n";
    // std::cerr << "vector storage: " << vector_size << " bytes\n";

    // std::ofstream myfile;
    // myfile.open("primary_key_count.csv");
    // for (auto const &pair: primary_key_count_to_occurrences) {
    //     uint64_t count = pair.second;
    //     while(count--){
    //         myfile << pair.first << std::endl;
    //     }
    //     // std::cout << "{" << pair.first << ": " << pair.second << "}\n";
    // }

    // myfile.open("primary_key_count.txt");
    // for (auto const &pair: primary_key_count_to_occurrences) {
    //     // uint64_t count = pair.second;
    //     // while(count--){
    //     // myfile << pair.first << std::endl;
    //     // }
    //     myfile << "{" << pair.first << ": " << pair.second << "}\n";
    // }    
    // myfile.close();

    // fprintf(stderr, "Average vector insertion time: %f, out of %ld times\n", (float)vector_time / vect_opt_count, vect_opt_count);
    // fprintf(stderr, "Total bimap insertion time: %lld, total bimap count: %ld\n", total_bimap, bimap_insertion_count);
    return;
    
}

void test_mdtrie_size(level_t max_depth, level_t trie_depth, preorder_t max_tree_node){
    
    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point();

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../libmdtrie/bench/data/sample_shuf.txt", "r");

    // If the file cannot be open
    if (fp == nullptr)
    {
        fprintf(stderr, "file not found\n");
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            printf("Current working dir: %s\n", cwd);
        } else {
            perror("getcwd() error");
        }
        exit(EXIT_FAILURE);
    }

    TimeStamp start, diff;
    
    n_leaves_t n_points = 0;
    n_leaves_t n_lines = 14583357;

    tqdm bar;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar.progress(n_points, n_lines);
        // Get the first token
        char *token = strtok(line, " ");
        char *ptr;
        // Skip the second and third token
        for (uint8_t i = 0; i < 2; i ++){
            strtok(nullptr, " ");
        }
        for (dimension_t i = 0; i < DIMENSION; i++){
            token = strtok(nullptr, " ");
            leaf_point->set_coordinate(i, strtoul(token, &ptr, 10));
        }
        start = GetTimestamp();
        mdtrie->insert_trie(leaf_point, max_depth, n_points);
        diff += GetTimestamp() - start;
        n_points ++;
    }
    bar.finish();
    fprintf(fptr, "Average time to insert one point: %f microseconds\n", (float)diff / n_points);
    fprintf(fptr, "Size of data structure: %ld bytes\n", mdtrie->size());
    // fprintf(fptr, "Size of dfuds: %ld bytes\n", dfuds_size);
}

void cdf_insert(level_t max_depth, level_t trie_depth, preorder_t max_tree_node){

    fptr = fopen(file_path_csv, "w");
    n_leaves_t n_lines = 14583357;
    // fprintf(fptr, "num_points, Latency\n");
    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point();

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../libmdtrie/bench/data/sample_shuf.txt", "r");

    // If the file cannot be open
    if (fp == nullptr)
    {
        fprintf(stderr, "file not found\n");
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            printf("Current working dir: %s\n", cwd);
        } else {
            perror("getcwd() error");
        }
        exit(EXIT_FAILURE);
    }
    TimeStamp latency, start;
    
    n_leaves_t n_points = 0;
    latency = 0;
    tqdm bar;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar.progress(n_points, n_lines);
        char *token = strtok(line, " ");
        char *ptr;
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
        n_points ++;
        start = GetTimestamp();
        mdtrie->insert_trie(leaf_point, max_depth, n_points);
        // latency += GetTimestamp() - start;
        latency = GetTimestamp() - start;

        fprintf(fptr, "%f\n", (float)latency);
    }
    bar.finish();
    fclose(fptr);
}

int main() {

    test_insert_data(32, 10, 1024);
    // test_random_data(50000, 32, 10, 1024);
}

