#include "trie.h"

#include <sys/time.h>

typedef unsigned long long int TimeStamp;
static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, NULL);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

bool test_random_data(int n_points, int dimensions)
{
    symbol_type n_branches = pow(2, dimensions);
    md_trie *mdtrie = new md_trie(dimensions);

    leaf_config *leaf_point = new leaf_config(dimensions);

    TimeStamp t0, t1;
    t0 = GetTimestamp();
    for (int itr = 1; itr <= n_points; itr ++){

        for (int i = 0; i < dimensions; i++){
            leaf_point->coordinates[i] = rand() % n_branches;
        }
        mdtrie->insert_trie(leaf_point, max_depth);
        if (!mdtrie->check(leaf_point, max_depth)){
            return false;
        }
    }

    t1 = GetTimestamp();
    fprintf(stderr, "Time to insert %d points with %d dimensions: %llu microseconds\n", n_points, dimensions, (t1 - t0));
    fprintf(stderr, "Average time to insert one point: %llu microseconds\n", (t1 - t0) / n_points);
    return true;
}

int main() {
    test_random_data(10000, 10);
}

