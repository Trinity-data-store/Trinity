#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>
#include <tqdm.h>

typedef unsigned long long int TimeStamp;
static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, NULL);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

void test_random_data(int n_points, int dimensions, level_type max_depth = 10)
{
    symbol_type range = pow(2, max_depth);
    md_trie *mdtrie = new md_trie(dimensions);

    leaf_config *leaf_point = new leaf_config(dimensions);

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
    fprintf(stderr, "Time to insert %d points with %d dimensions: %llu microseconds\n", n_points, dimensions, (t1 - t0));
    fprintf(stderr, "Average time to insert one point: %llu microseconds\n", (t1 - t0) / n_points);
}

void test_real_data(int dimensions, level_type max_depth){

    md_trie *mdtrie = new md_trie(dimensions, max_depth);    
    leaf_config *leaf_point = new leaf_config(dimensions);

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../libmdtrie/bench/data/sample.txt", "r");

    // If the file cannot be open
    if (fp == NULL)
    {
        fprintf(stderr, "file not found\n");
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Current working dir: %s\n", cwd);
        } else {
            perror("getcwd() error");
        }
        exit(EXIT_FAILURE);
    }

    TimeStamp start, diff;
    
    int n_points = 0;
    
    // Get the number of lines in the text file
    // int n_lines = 0;
    // char c;
    // for (c = getc(fp); c != EOF; c = getc(fp))
    //     if (c == '\n') 
    //         n_lines = n_lines + 1;
    // rewind(fp);
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
            token = strtok(NULL, " ");
        }
        for (int i = 0; i < dimensions; i++){
            token = strtok(NULL, " ");
            leaf_point->coordinates[i] = strtoul(token, &ptr, 10);;
        }
        start = GetTimestamp();
        mdtrie->insert_trie(leaf_point, max_depth);
        diff = GetTimestamp() - start;
        n_points ++;
    }
    bar.finish();
       
    fprintf(stderr, "Time to insert %d points with %d dimensions: %llu microseconds\n", n_points, dimensions, diff);
    fprintf(stderr, "Average time to insert one point: %llu microseconds\n", diff / n_points);

    // Query n_lines random points
    n_points = 0;
    diff = 0;
    symbol_type range = pow(2, max_depth);
    while (n_points < n_lines)
    {
        bar.progress(n_points, n_lines);
        // Get the first token
        for (int i = 0; i < dimensions; i++){
            leaf_point->coordinates[i] = rand() % range;
        }
        start = GetTimestamp();
        mdtrie->check(leaf_point, max_depth);
        diff += GetTimestamp() - start;
        n_points ++;
    }

    bar.finish();
    fprintf(stderr, "Time to query %d random points with %d dimensions: %llu microseconds\n", n_points, dimensions, diff);
    fprintf(stderr, "Average time to query one point: %llu microseconds\n", diff / n_points); 

    // Time to query the inserted points
    rewind(fp);
    n_points = 0;
    diff = 0;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        bar.progress(n_points, n_lines);
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
            fprintf(stderr, "error!\n");
        }
        diff += GetTimestamp() - start;
        n_points ++;
    } 
    bar.finish();
    fprintf(stderr, "Time to query %d inserted points with %d dimensions: %llu microseconds\n", n_points, dimensions, diff);
    fprintf(stderr, "Average time to query one point: %llu microseconds\n", diff / n_points);        
}

int main() {

    // test_random_data(10000, 10);

    test_real_data(2, 32); 

    test_real_data(3, 32);   

    test_real_data(4, 32);
    // 4 dimensions
    // Time to insert 14583357 points with 4 dimensions: 1917747475 microseconds (32 minutes)
    // Average time to insert one point: 131 microseconds 

  
}

