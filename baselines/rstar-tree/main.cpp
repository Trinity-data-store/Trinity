/*
 *  Copyright (c) 2008 Dustin Spicuzza <dustin@virtualroadside.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of version 2.1 of the GNU Lesser General Public
 *  License as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 * 
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

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

#define RANDOM_DATASET
const uint8_t DIMENSION = 7; // IMPORTANT TO SET!!!

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

void file_system()
{

	RTree tree;
	Visitor x;

	srand(time(0));
	#define nodes 14583357

    FILE *fp = fopen("../../data/fs/fs_dataset.txt", "r");

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

    /**
     * Insertion
     */

    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *token = strtok(line, " ");
        char *ptr;
		std::vector<int> coordinates(7, 0);
    
        for (uint8_t i = 1; i <= 2; i ++){
            token = strtok(nullptr, " ");
        }

        for (dimension_t i = 0; i < 7; i++){
            token = strtok(nullptr, " ");
            coordinates[i] = strtoul(token, &ptr, 10);
        }
        for (dimension_t i = 0; i < 7; i++){
            
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

		if (n_points % (nodes / 10) == 0)
			std::cout << "finished: " << n_points << std::endl;
    }
    std::cout << "insertion latency: " << (float) diff / n_points << std::endl;

    /**
     * Benchmark range search given a query selectivity (1000-2000), given a query
     */

    line = nullptr;
    len = 0;
    fp = fopen("../../queries/fs/fs_range_queries.csv", "r");
    int count = 0;
    diff = 0;
    read = getline(&line, &len, fp);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        std::vector<int> start_range(7, 0);
        std::vector<int> end_range(7, 0);

        char *ptr;
        char *token = strtok(line, ","); // id

        for (dimension_t i = 0; i < DIMENSION - 1; i++){
            token = strtok(nullptr, ","); // id
            start_range[i] = strtoul(token, &ptr, 10);
            token = strtok(nullptr, ",");
            end_range[i] = strtoul(token, &ptr, 10);
        }
        if (DIMENSION > 6){
            start_range[6] = min[6];
            end_range[6] = max[6];
        }

        // int present_pt_count = 0;
        // for (unsigned int i = 0; i < all_points.size(); i++){
        //     bool match = true;
        //     for (dimension_t j = 0; j < 7; j++){
        //         if (all_points[i][j] < start_range[j] || all_points[i][j] > end_range[j]){
        //             match = false;
        //             break;
        //         }
        //     }
        //     if (match){
        //         present_pt_count ++;
        //     }
        // }   
        // std::cout << "present point count: " << present_pt_count << std::endl;

        BoundingBox bound = bounds_range(start_range, end_range);
        start = GetTimestamp();
        x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;

        count ++;   
        std::cout << "x.count: " << x.count << std::endl; 
        std::cout << "diff: " << temp_diff << std::endl;
        // file_range_search << x.count << "," << temp_diff << std::endl; 
        leaf_list.clear();
    }
    std::cout << "average query latency: " << (float) diff / count << std::endl;    



    // std::vector<int> start_range(DIMENSION, 0);
    // std::vector<int> end_range(DIMENSION, 0);
    // for (int j = 0; j < DIMENSION; j++){
    //     start_range[j] = 0;
    //     end_range[j] = 2147483646;
    // }

    // BoundingBox bound = bounds_range(start_range, end_range);
    // start = GetTimestamp();
    // x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
    // diff = GetTimestamp() - start;

    // std::cout << "Found " << x.count << " nodes (" << tree.GetSize() << " nodes in tree)" << "latency: " << diff << std::endl;
    


	// std::cout << "latency per point: " << (float) diff / n_points << std::endl;
    // std::ofstream file("range_search_filesystem_r_star.csv", std::ios_base::app);

    /**
     * Benchmark range search given a query selectivity (1000-2000), find new query
     */


    // int itr = 0;
    // while (itr < 600){

    //     std::vector<int> start_range(DIMENSION, 0);
    //     std::vector<int> end_range(DIMENSION, 0);
    //     for (int j = 0; j < DIMENSION; j++){
    //         start_range[j] = min[j] + (max[j] - min[j] + 1) / 3 * (rand() % 3);
    //         end_range[j] = start_range[j] + (max[j] - start_range[j] + 1) / 3 * (rand() % 3);
    //     }

    //     BoundingBox bound = bounds_range(start_range, end_range);

    //     // std::cout << "Searching in " << bound.ToString() << std::endl;
    //     start = GetTimestamp();
    //     x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
    //     diff = GetTimestamp() - start;

    //     // std::cout << "Found " << x.count << " nodes (" << tree.GetSize() << " nodes in tree)" << std::endl;

    //     if (x.count > 0){
    //         file << x.count << "," << diff << "," << std::endl;
    //         itr ++;
    //     }
    // }    
    

}

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

		std::vector<int> coordinates(4, 0);

        for (dimension_t i = 0; i < 4; i++){
            token = strtok(nullptr, ",");
            coordinates[i] = strtoul(token, &ptr, 10);
        }
        for (dimension_t i = 0; i < 4; i++){
            
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
    // std::ofstream file_range_search("osm_rstar_queries_1000.csv");
    // int next = 1;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        std::vector<int> start_range(4, 0);
        std::vector<int> end_range(4, 0);

        char *ptr;
        char *token = strtok(line, ","); // id
        token = strtok(nullptr, ",");
        token = strtok(nullptr, ",");

        for (dimension_t i = 0; i < 4; i++){
            token = strtok(nullptr, ","); // id
            start_range[i] = strtoul(token, &ptr, 10);
            token = strtok(nullptr, ",");
            end_range[i] = strtoul(token, &ptr, 10);
        }
        count ++;   
        // int present_pt_count = 0;
        // for (unsigned int i = 0; i < all_points.size(); i++){
        //     bool match = true;
        //     for (dimension_t j = 0; j < 4; j++){
        //         if ( all_points[i][j] < start_range[j] || all_points[i][j] > end_range[j]){
        //             match = false;
        //             break;
        //         }
        //     }
        //     if (match){
        //         present_pt_count ++;
        //     }
        // }   
        // std::cout << "present point count: " << present_pt_count << std::endl;

        BoundingBox bound = bounds_range(start_range, end_range);
        start = GetTimestamp();
        x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;

        // std::cout << "x.count: " << x.count << std::endl; 
        // std::cout << "leaf_list: " << leaf_list.size() << std::endl;
        // std::cout << "diff: " << temp_diff << std::endl;
        // file_range_search << x.count << "," << temp_diff << std::endl; 
        leaf_list.clear();
    }
    std::cout << "average query latency: " << (float) diff / count << std::endl;    

    // exit(0);

    // std::vector<int> start_range(4, 0);
    // std::vector<int> end_range(4, 0);

    // for (int j = 0; j < 4; j++){
    //     start_range[j] = 0;
    //     end_range[j] = 2147483646;
    // }

    // BoundingBox bound = bounds_range(start_range, end_range);
    // start = GetTimestamp();
    // x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
    // diff = GetTimestamp() - start;
    
    // std::cout << "vector size: " << leaf_list.size() << std::endl;
    // std::cout << "Found " << x.count << " nodes (" << tree.GetSize() << " nodes in tree)" << "latency: " << diff << std::endl;
    
    // exit(0);
    
	// std::cout << "latency per point: " << (float) diff / n_points << std::endl;
    // std::ofstream file("range_search_osm_r_star.csv", std::ios_base::app);

    // int itr = 0;
    // while (itr < 600){

    //     std::vector<int> start_range(4, 0);
    //     std::vector<int> end_range(4, 0);
    //     for (int j = 0; j < 4; j++){
    //         start_range[j] = min[j] + (max[j] - min[j] + 1) / 3 * (rand() % 3);
    //         end_range[j] = start_range[j] + (max[j] - start_range[j] + 1) / 3 * (rand() % 3);
    //     }

    //     BoundingBox bound = bounds_range(start_range, end_range);

    //     start = GetTimestamp();
    //     x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
    //     diff = GetTimestamp() - start;

    //     if (x.count > 0){
    //         file << x.count << "," << diff << "," << std::endl;
    //         itr ++;
    //     }
    // }    

	// return 0;
}

void tpch()
{
	RTree tree;
	Visitor x;

	srand(time(0));
	#define nodes_tpch 300005812

    std::ifstream infile("/../../data/tpc-h/tpch_dataset.csv");

    TimeStamp start, diff;
    diff = 0;
    uint64_t n_points = 0;
    uint64_t max[9];
    uint64_t min[9];
    std::string line;
    std::getline(infile, line);
    // std::vector<std::vector<int>> all_points;

    /**
     * Insertion
     */

    while (std::getline(infile, line))
    {
		std::vector<int> coordinates(9, 0);

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

        for (dimension_t i = 0; i < 9; i++){
            
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

        // all_points.push_back(coordinates);
        start = GetTimestamp();
        tree.Insert(n_points, bounds(coordinates));
        diff += GetTimestamp() - start;
        n_points ++;

		if (n_points % (nodes_tpch / 10) == 0)
			std::cout << "finished: " << n_points << std::endl;
        
    }

    std::cout << "insertion latency: " << (float) diff / n_points << std::endl;

    /**
     * Benchmark range search given a query selectivity (1000-2000), given a query
     */

    char *line_query = nullptr;
    size_t len = 0;
    FILE *fp = fopen("../../queries/tpc-h/tpch_dataset.csv", "r");
    int count = 0;
    diff = 0;
    // std::ofstream file_range_search("tpch_rstar_queries_1000.csv");
    ssize_t read = getline(&line_query, &len, fp);

    while ((read = getline(&line_query, &len, fp)) != -1)
    {
        std::vector<int> start_range(9, 0);
        std::vector<int> end_range(9, 0);

        char *ptr;
        char *token = strtok(line_query, ","); // id
        // token = strtok(nullptr, ",");
        // token = strtok(nullptr, ",");

        for (dimension_t i = 0; i < 9; i++){
            token = strtok(nullptr, ","); // id
            start_range[i] = strtoul(token, &ptr, 10);
            token = strtok(nullptr, ",");
            end_range[i] = strtoul(token, &ptr, 10);
        }

        // int present_pt_count = 0;
        // for (unsigned int i = 0; i < all_points.size(); i++){
        //     bool match = true;
        //     for (dimension_t j = 0; j < 9; j++){
        //         if ( all_points[i][j] < start_range[j] || all_points[i][j] > end_range[j]){
        //             match = false;
        //             break;
        //         }
        //     }
        //     if (match){
        //         present_pt_count ++;
        //     }
        // }   
        // std::cout << "present point count: " << present_pt_count << std::endl;

        BoundingBox bound = bounds_range(start_range, end_range);
        start = GetTimestamp();
        x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;

        count ++;   
        // std::cout << "x.count: " << x.count << std::endl; 
        // std::cout << "diff: " << temp_diff << std::endl;
        // file_range_search << x.count << "," << temp_diff << std::endl; 
        leaf_list.clear();
    }
    std::cout << "average query latency: " << (float) diff / count << std::endl;    

    // exit(0);

    /*
    char *line_range = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("/home/ziming/phtree-cpp/build/tpch_phtree_queries_1000.csv", "r");
    int count = 0;
    diff = 0;
    std::ofstream file_range_search("tpch_rstar_queries_1000.csv");

    while ((read = getline(&line_range, &len, fp)) != -1)
    {
        std::vector<int> start_range(9, 0);
        std::vector<int> end_range(9, 0);

        char *ptr;
        char *token = strtok(line_range, ","); // id
        token = strtok(nullptr, ",");
        token = strtok(nullptr, ",");

        for (dimension_t i = 0; i < 9; i++){
            token = strtok(nullptr, ","); // id
            start_range[i] = strtoul(token, &ptr, 10);
            token = strtok(nullptr, ",");
            end_range[i] = strtoul(token, &ptr, 10);
        }
        
        int present_pt_count = 0;
        for (unsigned int i = 0; i < all_points.size(); i++){
            bool match = true;
            for (dimension_t j = 0; j < 9; j++){
                if (all_points[i][j] < start_range[j] || all_points[i][j] > end_range[j]){
                    match = false;
                    break;
                }
            }
            if (match){
                present_pt_count ++;
            }
        }   
        std::cout << "present point count: " << present_pt_count << std::endl;

        BoundingBox bound = bounds_range(start_range, end_range);
        start = GetTimestamp();
        x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;

        count ++;   
        std::cout << "x.count: " << x.count << std::endl; 
        std::cout << "diff: " << temp_diff << std::endl;
        file_range_search << x.count << "," << temp_diff << std::endl; 
        leaf_list.clear();
    }
    std::cout << "average query latency: " << (float) diff / count << std::endl;    

    */
    // exit(0);

    // std::ofstream file("tpch_rstar_queries_1000.csv", std::ios_base::app);
    // srand(time(NULL));

    // int itr = 0;
    // while (itr < 600){

    //     std::vector<int> start_range(9, 0);
    //     std::vector<int> end_range(9, 0);
    //     for (int j = 0; j < 9; j++){
    //         start_range[j] = min[j] + rand() % ((uint32_t) (max[j] - min[j]));            
    //         end_range[j] = start_range[j] + (max[j] - start_range[j] + 1) / 20 * (rand() % 20);
    //     }

    //     BoundingBox bound = bounds_range(start_range, end_range);

    //     start = GetTimestamp();
    //     x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
    //     diff = GetTimestamp() - start;

    //     if (x.count >= 1000 && x.count <= 2000){
            
    //         int present_pt_count = 0;
    //         for (unsigned int i = 0; i < all_points.size(); i++)
    //         {
    //             bool match = true;
    //             for (unsigned int j = 0; j < 9; j++){
    //                 if (all_points[i][j] < start_range[j] || all_points[i][j] > end_range[j]){
    //                     match = false;
    //                     break;
    //                 }
    //             }
    //             if (match)
    //                 present_pt_count ++;                
    //         }

    //         if (present_pt_count < 1000 || present_pt_count > 2000)
    //             continue;

    //         std::cout << "present point count: " << present_pt_count << " x.count: "<< x.count << std::endl;

    //         file << x.count << "," << diff << "," << "search_range,";

    //         for (uint8_t j = 0; j < 9; j ++)
    //         {
    //             file << start_range[j] << "," << end_range[j] << ",";
    //         }        
    //         file << std::endl;    
    //         itr ++;
    //         std::cout << "Found a query!" << std::endl;
    //     }
    // }    



    // exit(0);

/*
    std::vector<int> start_range(9, 0);
    std::vector<int> end_range(9, 0);
    for (int j = 0; j < 9; j++){
        start_range[j] = 0;
        end_range[j] = 2147483646;
    }

    BoundingBox bound = bounds_range(start_range, end_range);
    start = GetTimestamp();
    x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
    diff = GetTimestamp() - start;

    std::cout << "Found " << x.count << " nodes (" << tree.GetSize() << " nodes in tree)" << "latency: " << diff << std::endl;
    

	std::cout << "latency per point: " << (float) diff / n_points << std::endl;
    std::ofstream file("range_search_tpch_r_star.csv", std::ios_base::app);

    int itr = 0;
    while (itr < 600){

        std::vector<int> start_range(9, 0);
        std::vector<int> end_range(9, 0);
        for (int j = 0; j < 9; j++){
            start_range[j] = min[j] + (max[j] - min[j] + 1) / 3 * (rand() % 3);
            end_range[j] = start_range[j] + (max[j] - start_range[j] + 1) / 3 * (rand() % 3);
        }

        BoundingBox bound = bounds_range(start_range, end_range);

        start = GetTimestamp();
        x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
        diff = GetTimestamp() - start;

        if (x.count > 0){
            file << x.count << "," << diff << "," << std::endl;
            itr ++;
        }
    }    
*/
}

int main(int argc, char *argv[]){

    if (argc != 2){
        std::cerr << "wrong number of input!" << std::endl;
        exit(0);
    }
    
    int dataset_index = atoi(argv[1]);

    if (dataset_index == 0){
        std::cout << "file system......" << std::endl;
        file_system();
    }
    if (dataset_index == 1){
        std::cout << "osm......" << std::endl;
        osm();
    }
    if (dataset_index == 2){
        std::cout << "tpch......" << std::endl;
        tpch();
    }
}