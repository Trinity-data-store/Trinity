#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <tqdm.h>

FILE *fptr;
char file_path[] = "benchmark_output_nonexistent.txt";

typedef unsigned long long int TimeStamp;
static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

void test_random_data(int n_points, int dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_node)
{
    auto range = (symbol_type)pow(2, max_depth);
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_node);

    auto *leaf_point = new leaf_config(dimensions);

    TimeStamp t0, t1;
    t0 = GetTimestamp();
    for (int itr = 1; itr <= n_points; itr ++){

        for (int i = 0; i < dimensions; i++){
            leaf_point->coordinates[i] = rand() % range;
        }
        mdtrie->insert_trie(leaf_point, max_depth);
        if (!mdtrie->check(leaf_point, max_depth)){
            fprintf(stderr, "Error insertion!\n");
        }
    }

    t1 = GetTimestamp();
    fprintf(fptr, "Time to insert %d points with %d dimensions: %llu microseconds\n", n_points, dimensions, (t1 - t0));
    fprintf(fptr, "Average time to insert one point: %llu microseconds\n", (t1 - t0) / n_points);
}

void test_real_data(int dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_node){

    fptr = fopen(file_path, "a");
    fprintf(fptr, "dimensions: %d, trie_depth: %ld, max_tree_node: %ld\n", dimensions, trie_depth, max_tree_node);
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new leaf_config(dimensions);

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
    
    int n_points = 0;

    int n_lines = 14583357;
    diff = 0;
    tqdm bar1;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar1.progress(n_points, n_lines);
        // Get the first token
        char *token = strtok(line, " ");
        char *ptr;
        // Skip the second and third token
        for (int i = 0; i < 2; i ++){
            token = strtok(nullptr, " ");
        }
        for (int i = 0; i < dimensions; i++){
            token = strtok(nullptr, " ");
            leaf_point->coordinates[i] = strtoul(token, &ptr, 10);;
        }
        start = GetTimestamp();
        mdtrie->insert_trie(leaf_point, max_depth);
        diff += GetTimestamp() - start;
        n_points ++;
    }
    bar1.finish();
    uint64_t msec = diff * 1000 / CLOCKS_PER_SEC;
    fprintf(fptr, "Average time to insert one point: %f microseconds per insertion\n", (float)msec*1000 / n_points);
    fprintf(fptr, "Size of data structure that contains %d points: %ld bytes\n", n_lines, mdtrie->size());

    // Query n_lines random points
    n_points = 0;
    diff = 0;
    int n_nonexistent = 0;
    tqdm bar2;
    auto range = (symbol_type)pow(2, max_depth);
    while (n_points < n_lines)
    {
        bar2.progress(n_points, n_lines);
        // Get the first token
        for (int i = 0; i < dimensions; i++){
            leaf_point->coordinates[i] = rand() % range;
        }
        start = GetTimestamp();
        if(!mdtrie->check(leaf_point, max_depth)){
            diff += GetTimestamp() - start;
            n_nonexistent += 1;
        }
        
        n_points ++;
    }
    bar2.finish();
    msec = diff * 1000 / CLOCKS_PER_SEC;
    fprintf(fptr, "Average time to query nonexistent points: %f microseconds per query\n", (float)msec*1000 / n_nonexistent); 
    fprintf(fptr, "\n");
    fclose(fptr);
    return

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
        for (int i = 0; i < 2; i ++){
            token = strtok(NULL, " ");
        }
        for (int i = 0; i < dimensions; i++){
            token = strtok(NULL, " ");
            leaf_point->coordinates[i] = strtoul(token, &ptr, 10);;
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
    msec = diff * 1000 / CLOCKS_PER_SEC;
    fprintf(fptr, "Average time to query points that are in the trie: %f microseconds per query\n", (float)msec*1000 / n_points);
    fprintf(fptr, "\n");
    fclose(fptr);
}

void test_insert_data(int dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_node){

    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new leaf_config(dimensions);

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
    
    int n_points = 0;
    int n_lines = 14583357;

    tqdm bar;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar.progress(n_points, n_lines);
        // Get the first token
        char *token = strtok(line, " ");
        char *ptr;
        // Skip the second and third token
        for (int i = 0; i < 2; i ++){
            strtok(nullptr, " ");
        }
        for (int i = 0; i < dimensions; i++){
            token = strtok(nullptr, " ");
            leaf_point->coordinates[i] = strtoul(token, &ptr, 10);;
        }
        start = GetTimestamp();
        mdtrie->insert_trie(leaf_point, max_depth);
        diff += GetTimestamp() - start;
        n_points ++;
    }
    bar.finish();
    uint64_t msec = diff * 1000 / CLOCKS_PER_SEC;
    fprintf(fptr, "Average time to insert one point: %f microseconds\n", (float)msec*1000 / n_points);
}


void test_mdtrie_size(int dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_node){
    
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new leaf_config(dimensions);

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
    
    int n_points = 0;
    int n_lines = 14583357;

    tqdm bar;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar.progress(n_points, n_lines);
        // Get the first token
        char *token = strtok(line, " ");
        char *ptr;
        // Skip the second and third token
        for (int i = 0; i < 2; i ++){
            strtok(nullptr, " ");
        }
        for (int i = 0; i < dimensions; i++){
            token = strtok(nullptr, " ");
            leaf_point->coordinates[i] = strtoul(token, &ptr, 10);;
        }
        start = GetTimestamp();
        mdtrie->insert_trie(leaf_point, max_depth);
        diff += GetTimestamp() - start;
        n_points ++;
    }
    bar.finish();
    uint64_t msec = diff * 1000 / CLOCKS_PER_SEC;
    fprintf(fptr, "Average time to insert one point: %f microseconds\n", (float)msec*1000 / n_points);
    fprintf(fptr, "Size of data structure: %ld bytes\n", mdtrie->size());
    fprintf(fptr, "Size of dfuds: %ld bytes\n", dfuds_size);
}


int main() {

//  int dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_node
    test_real_data(2, 32, 10, 1024);
    test_real_data(2, 32, 6, 1024);
    test_real_data(2, 32, 8, 1024);
    test_real_data(2, 32, 12, 1024);
    test_real_data(2, 32, 14, 1024);
    test_real_data(2, 32, 16, 1024);
    test_real_data(4, 32, 10, 1024);
    test_real_data(3, 32, 10, 1024);

}

