#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <tqdm.h>

FILE *fptr;
char file_path[] = "benchmark_range_search_3d.csv";

typedef unsigned long long int TimeStamp;
static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

void test_real_data(dimension_t dimensions, level_t max_depth, level_t trie_depth, preorder_t max_tree_node, int n_itr){

    fptr = fopen(file_path, "a");
    fprintf(fptr, "Number of Points, Volume, Latency, %d\n", dimensions);
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point(dimensions);

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
    auto max = (uint64_t *)malloc(dimensions * sizeof(uint64_t));
    auto min = (uint64_t *)malloc(dimensions * sizeof(uint64_t));
    n_leaves_t n_lines = 14583357;
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
        for (dimension_t i = 0; i < dimensions; i++){
            token = strtok(nullptr, " ");
            leaf_point->set_coordinate(i, strtoul(token, &ptr, 10));
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
    bar.finish();
    uint64_t msec;

    
    auto *start_range = new data_point(dimensions);
    auto *end_range = new data_point(dimensions);
    auto *found_points = new point_array();
    // uint8_t *representation = (uint8_t *)malloc(sizeof(uint8_t) * dimensions);
    int itr = 1;
    tqdm bar1;

    while (itr <= n_itr){
        bar1.progress(itr, n_itr);
        uint64_t volume = 1;
        for (dimension_t i = 0; i < dimensions; i++){
            start_range->set_coordinate(i,  min[i] + rand() % (max[i] - min[i] + 1));
            end_range->set_coordinate(i, start_range->get_coordinate(i) + rand() % (max[i] - start_range->get_coordinate(i) + 1));
            volume *= (end_range->get_coordinate(i) - start_range->get_coordinate(i));
        }
        start = GetTimestamp();
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
        diff = GetTimestamp() - start;
        if (found_points->size() == 0){
            continue;
        }

        msec = diff * 1000 / CLOCKS_PER_SEC;
        fprintf(fptr, "%ld, %ld, %f\n", found_points->size(), volume, (float)msec*1000);
        
        found_points->reset();
        itr++;
        fclose(fptr);
        fptr = fopen(file_path, "a");            
    }
    bar1.finish();
    fclose(fptr);
}

void range_search_insert(md_trie *mdtrie, dimension_t dimensions, level_t max_depth, uint64_t * max, uint64_t *min){
    auto *leaf_point = new data_point(dimensions);

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
    // TimeStamp start, diff;
    
    n_leaves_t n_points = 0;
    // auto max = (uint64_t *)malloc(dimensions * sizeof(uint64_t));
    // auto min = (uint64_t *)malloc(dimensions * sizeof(uint64_t));
    n_leaves_t n_lines = 14583357;
    // diff = 0;
    tqdm bar;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar.progress(n_points, n_lines);
        char *token = strtok(line, " ");
        char *ptr;
        for (uint8_t i = 0; i < 2; i ++){
            token = strtok(nullptr, " ");
        }
        for (dimension_t i = 0; i < dimensions; i++){
            token = strtok(nullptr, " ");
            leaf_point->set_coordinate(i, strtoul(token, &ptr, 10));
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
    bar.finish();
}
void test_range_only(dimension_t dimensions, level_t max_depth, level_t trie_depth, preorder_t max_tree_node, int n_itr){

    // fptr = fopen(file_path, "a");
    // fprintf(fptr, "Number of Points, Volume, Latency, %d\n", dimensions);
    auto max = (uint64_t *)malloc(dimensions * sizeof(uint64_t));
    auto min = (uint64_t *)malloc(dimensions * sizeof(uint64_t));
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_node);
    range_search_insert(mdtrie, dimensions, max_depth, max, min);


    // uint64_t msec;    
    auto *start_range = new data_point(dimensions);
    auto *end_range = new data_point(dimensions);
    auto *found_points = new point_array();
    // uint8_t *representation = (uint8_t *)malloc(sizeof(uint8_t) * dimensions);
    int itr = 1;
    // tqdm bar1;
    // TimeStamp start, diff;
    while (itr <= n_itr){
        // bar1.progress(itr, n_itr);
        uint64_t volume = 1;
        for (dimension_t i = 0; i < dimensions; i++){
            start_range->set_coordinate(i,  min[i] + rand() % (max[i] - min[i] + 1));
            end_range->set_coordinate(i, start_range->get_coordinate(i) + rand() % (max[i] - start_range->get_coordinate(i) + 1));
            volume *= (end_range->get_coordinate(i) - start_range->get_coordinate(i));
        }
        // start = GetTimestamp();
        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
        // diff = GetTimestamp() - start;
        if (found_points->size() == 0){
            continue;
        }

        // msec = diff * 1000 / CLOCKS_PER_SEC;
        // fprintf(fptr, "%ld, %ld, %f\n", found_points->size(), volume, (float)msec*1000);
        
        found_points->reset();
        itr++;
        // fclose(fptr);
        // fptr = fopen(file_path, "a");            
    }

    // bar1.finish();
    // fclose(fptr);
}

bool test_random_range_search(n_leaves_t n_points, dimension_t dimensions, level_t max_depth, level_t trie_depth,
                       preorder_t max_tree_nodes, uint32_t n_itr = 50) {
    auto range = (symbol_t) pow(2, max_depth);
    auto *mdtrie = new md_trie(dimensions, max_depth, trie_depth, max_tree_nodes);
    auto *leaf_point = new data_point(dimensions);
    auto max = (uint64_t *) malloc(dimensions * sizeof(uint64_t));
    auto min = (uint64_t *) malloc(dimensions * sizeof(uint64_t));

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {

        for (dimension_t i = 0; i < dimensions; i++) {
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
        mdtrie->insert_trie(leaf_point, max_depth);
    }
    auto *start_range = new data_point(dimensions);
    auto *end_range = new data_point(dimensions);
    auto *found_points = new point_array();
    // tqdm bar1;
    for (uint32_t itr = 1; itr <= n_itr; itr++) {
        // bar1.progress(itr, n_itr);
        for (dimension_t i = 0; i < dimensions; i++) {
            start_range->set_coordinate(i, min[i] + rand() % (max[i] - min[i] + 1));
            end_range->set_coordinate(i, start_range->get_coordinate(i) + rand() % (max[i] - start_range->get_coordinate(i) + 1));
        }

        mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
        found_points->reset();
    }
    // bar1.finish();
    return true;
}

// int dimensions, level_type max_depth, level_type trie_depth, preorder_type max_tree_node, int n_itr
int main() {
    srand(static_cast<unsigned int>(time(0)));
    
    test_real_data(3, 32, 10, 1024, 50);
    // test_range_only(3, 32, 10, 1024, 50);
    // test_real_data(4, 32, 10, 1024, 200);

    // test_random_range_search(50000, 3, 10, 3, 1024, 200);

}