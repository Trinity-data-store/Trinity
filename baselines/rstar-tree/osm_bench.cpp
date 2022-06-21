#include <cinttypes>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <string.h>

#include <stdio.h>
#include "RStarTree.h"
#include <vector>

#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>

#define RANDOM_DATASET
const uint8_t DIMENSION = 4; 

typedef RStarTree<int, DIMENSION, 32, 64> RTree;

typedef RTree::BoundingBox BoundingBox;

typedef uint64_t dimension_t;

typedef unsigned long long int TimeStamp;

TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

BoundingBox bounds(std::vector<int> coordinates)
{
	BoundingBox bb;
	
	for (uint8_t i = 0; i < DIMENSION; i++){
		bb.edges[i].first = coordinates[i];
		bb.edges[i].second = coordinates[i];
	}
	return bb;
}

BoundingBox bounds_range(std::vector<int> start, std::vector<int> end)
{
	BoundingBox bb;
	
	for (uint8_t i = 0; i < DIMENSION; i++){
		bb.edges[i].first = start[i];
		bb.edges[i].second = end[i];
	}
	return bb;
}

std::vector<RTree::Leaf *> leaf_list;
struct Visitor {
	int count;
	bool ContinueVisiting;
	
	Visitor() : count(0), ContinueVisiting(true) {};
	
	void operator()(RTree::Leaf * leaf) 
	{
		count++;
        leaf_list.push_back(leaf);
	}
};

void osm()
{
	RTree tree;
	Visitor x;

	srand(time(0));
    const int num_nodes = 152806264;

    FILE *fp = fopen("../../data/osm/osm_dataset.csv", "r");

    // If the file cannot be open
    if (fp == nullptr)
    {
        fprintf(stderr, "file not found\n");
        exit(EXIT_FAILURE);
    }

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    TimeStamp start, diff;
    diff = 0;
    uint64_t n_points = 0;
    uint64_t max[DIMENSION];
    uint64_t min[DIMENSION];
    read = getline(&line, &len, fp);

    /**
     * Insertion
     */

    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *token = strtok(line, ","); // id
        char *ptr;

		std::vector<int> coordinates(DIMENSION, 0);

        for (dimension_t i = 0; i < DIMENSION; i++){
            token = strtok(nullptr, ",");
            coordinates[i] = strtoul(token, &ptr, 10);
        }
        for (dimension_t i = 0; i < DIMENSION; i++){
            
            if (n_points == 0){
                max[i] = coordinates[i];
                min[i] = coordinates[i];
            }
            else {
                if (coordinates[i] > max[i]){
                    max[i] = coordinates[i];
                }
                if (coordinates[i] < min[i]){
                    min[i] = coordinates[i];
                }
            }          
        }

        start = GetTimestamp();
        tree.Insert(n_points, bounds(coordinates));
        diff += GetTimestamp() - start;
        n_points ++;

		if (n_points % (num_nodes / 10) == 0)
			std::cout << "finished: " << n_points << std::endl;
        
        if (n_points == num_nodes)
            break;
    }
    
    std::cout << "insertion latency: " << (float) diff / n_points << std::endl;

    /**
     * Benchmark range search given a query selectivity (1000-2000), given a query
     */

    line = nullptr;
    len = 0;
    fp = fopen("../../queries/osm/osm_range_queries.csv", "r");
    int count = 0;
    diff = 0;
    read = getline(&line, &len, fp);
    std::vector<TimeStamp> latency_vect;

    while ((read = getline(&line, &len, fp)) != -1)
    {
        std::vector<int> start_range(DIMENSION, 0);
        std::vector<int> end_range(DIMENSION, 0);

        char *ptr;
        char *token = strtok(line, ","); // id

        for (dimension_t i = 0; i < DIMENSION; i++){
            token = strtok(nullptr, ","); // id
            start_range[i] = strtoul(token, &ptr, 10);
            token = strtok(nullptr, ",");
            end_range[i] = strtoul(token, &ptr, 10);
        }
        count ++;   

        BoundingBox bound = bounds_range(start_range, end_range);
        start = GetTimestamp();
        x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;
        latency_vect.push_back(temp_diff);

        leaf_list.clear();
    }
    std::cout << "average query latency: " << (float) diff / count << std::endl;    
    TimeStamp squared_cumulative = 0;
    for (unsigned int i = 0; i < latency_vect.size(); i++){
        squared_cumulative += (latency_vect[i] - diff / count) * (latency_vect[i] - diff / count);
    }
    std::cout << "Standard Deviation: " << (float) sqrt (squared_cumulative / (count - 1)) << std::endl;
}

int main(){

    std::cout << "osm......" << std::endl;
    osm();
}