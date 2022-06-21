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
const uint8_t DIMENSION = 9; 

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

void tpch()
{
	RTree tree;
	Visitor x;

	srand(time(0));
	#define nodes_tpch 300005812

    std::ifstream infile("../../data/tpc-h/tpch_dataset.csv");

    TimeStamp start, diff;
    diff = 0;
    uint64_t n_points = 0;
    uint64_t max[DIMENSION];
    uint64_t min[DIMENSION];
    std::string line;
    std::getline(infile, line);

    /**
     * Insertion
     */

    while (std::getline(infile, line))
    {
		std::vector<int> coordinates(DIMENSION, 0);

        std::stringstream ss(line);
        std::vector<std::string> string_result;

        // Parse string by ","
        int leaf_point_index = 0;
        int index = -1;

        // Kept indexes: 
        // [4, 5, 6, 7, 10, 11, 12, 16, 17]
        // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
        while (ss.good())
        {
            index ++;
            std::string substr;
            std::getline(ss, substr, ',');
        
            uint32_t num;
            if (index == 5 || index == 6 || index == 7 || index == 16) // float with 2dp
            {
                num = static_cast<uint32_t>(std::stof(substr) * 100);
            }
            else if (index == 10 || index == 11 || index == 12 || index == 17) //yy-mm-dd
            {
                substr.erase(std::remove(substr.begin(), substr.end(), '-'), substr.end());
                num = static_cast<uint32_t>(std::stoul(substr));
            }
            else if (index == 8 || index == 9 || index == 13 || index == 15 || index == 18) //skip text
                continue;
            else if (index == 0 || index == 1 || index == 2 || index == 14) // secondary keys
                continue;
            else if (index == 19) // all 0
                continue;
            else if (index == 3) // lineitem
                continue;
            else
                num = static_cast<uint32_t>(std::stoul(substr));

            coordinates[leaf_point_index] = num;
            leaf_point_index++;
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

		if (n_points % (nodes_tpch / 10) == 0)
			std::cout << "finished: " << n_points << std::endl;
    
        if (n_points == nodes_tpch)
            break;
        
    }

    std::cout << "insertion latency: " << (float) diff / n_points << std::endl;

    /**
     * Benchmark range search given a query selectivity (1000-2000), given a query
     */

    char *line_query = nullptr;
    size_t len = 0;
    FILE *fp = fopen("../../queries/tpch/tpch_range_queries.csv", "r");
    int count = 0;
    diff = 0;
    ssize_t read = getline(&line_query, &len, fp);
    std::vector<TimeStamp> latency_vect;

    while ((read = getline(&line_query, &len, fp)) != -1)
    {
        std::vector<int> start_range(DIMENSION, 0);
        std::vector<int> end_range(DIMENSION, 0);

        char *ptr;
        char *token = strtok(line_query, ","); // id

        for (dimension_t i = 0; i < DIMENSION; i++){
            token = strtok(nullptr, ","); // id
            start_range[i] = strtoul(token, &ptr, 10);
            token = strtok(nullptr, ",");
            end_range[i] = strtoul(token, &ptr, 10);
        }

        BoundingBox bound = bounds_range(start_range, end_range);
        start = GetTimestamp();
        x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;
        latency_vect.push_back(temp_diff);

        count ++;   
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
    std::cout << "tpch......" << std::endl;
    tpch();
}