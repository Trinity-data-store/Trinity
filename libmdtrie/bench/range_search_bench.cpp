#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <tqdm.h>

FILE *fptr;
char file_path[] = "benchmark_range_search.csv";

typedef unsigned long long int TimeStamp;
static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

void test_real_data(dimension_type dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_node, int n_itr){

    fptr = fopen(file_path, "a");
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
    
    n_leaves_type n_points = 0;
    auto max = (uint64_t *)malloc(dimensions * sizeof(uint64_t));
    auto min = (uint64_t *)malloc(dimensions * sizeof(uint64_t));
    n_leaves_type n_lines = 14583357;
    diff = 0;
    tqdm bar;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar.progress(n_points, n_lines);
        char *token = strtok(line, " ");
        char *ptr;
        for (uint8_t i = 0; i < 2; i ++){
            token = strtok(nullptr, " ");
        }
        for (dimension_type i = 0; i < dimensions; i++){
            token = strtok(nullptr, " ");
            leaf_point->coordinates[i] = strtoul(token, &ptr, 10);
            if (n_points == 0){
                max[i] = leaf_point->coordinates[i];
                min[i] = leaf_point->coordinates[i];
            }
            else {
                if (leaf_point->coordinates[i] > max[i]){
                    max[i] = leaf_point->coordinates[i];
                }
                if (leaf_point->coordinates[i] < min[i]){
                    min[i] = leaf_point->coordinates[i];
                }
            }
        }
        mdtrie->insert_trie(leaf_point, max_depth);
        n_points ++;
    }
    bar.finish();
    uint64_t msec;

    auto *start_range = new leaf_config(dimensions);
    auto *end_range = new leaf_config(dimensions);
    auto *found_points = new leaf_array();
    int itr = 1;
    tqdm bar1;
    while (itr <= n_itr){
        bar1.progress(itr, n_itr);
        for (dimension_type i = 0; i < dimensions; i++){
            start_range->coordinates[i] = min[i] + rand() % (max[i] - min[i] + 1);
            end_range->coordinates[i] = start_range->coordinates[i] + rand() % (max[i] - start_range->coordinates[i] + 1);
        }
        start = GetTimestamp();
        mdtrie->range_search_trie(start_range, end_range, mdtrie->get_root(), 0, found_points); 
        diff = GetTimestamp() - start;
        if (found_points->n_points == 0){
            continue;
        }

        msec = diff * 1000 / CLOCKS_PER_SEC;
        fprintf(fptr, "%ld, %f\n", found_points->n_points, (float)msec*1000);
        
        found_points->reset();
        itr++;
        fclose(fptr);
        fptr = fopen(file_path, "a");            
    }
    bar1.finish();
    fclose(fptr);
}

// int dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_node, int n_itr
int main() {
    srand(static_cast<unsigned int>(time(0)));
    test_real_data(4, 32, 10, 1024, 100);
}