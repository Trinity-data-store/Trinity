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
#include "./micro_common.hpp"

#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>

#define GITHUB_SIZE 200000000 // 200M
#define SKIP_SIZE 700000000 - GITHUB_SIZE
#define SERVER_TO_SERVER_IN_NS 92
#define DIMENSION 10 + 1

int points_to_insert = 30000;
int points_to_lookup = 30000;
int points_for_warmup = points_to_insert / 5;
bool dummy = true;
typedef RStarTree<bool, DIMENSION, 32, 64> RTree;

typedef RTree::BoundingBox BoundingBox;

typedef uint64_t dimension_t;

typedef unsigned long long int TimeStamp;

TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

// Parse one line from TPC-H file.
std::vector<int32_t>
parse_line_github(std::string line)
{

  std::vector<int32_t> point(DIMENSION - 1, 0);
  int index = -1;
  bool primary_key = true;
  std::string delim = ",";
  auto start = 0U;
  auto end = line.find(delim);
  // int real_index = -1;
  // [id, events_count, authors_count, forks, stars, issues, pushes, pulls,
  // downloads, adds, dels, add_del_ratio, start_date, end_date]
  while (end != std::string::npos) {
    std::string substr = line.substr(start, end - start);
    start = end + 1;
    end = line.find(delim, start);

    if (primary_key) {
      primary_key = false;
      continue;
    }
    index++;
    point[index] = static_cast<int32_t>(std::stoul(substr));
  }
  index++;
  std::string substr = line.substr(start, end - start);
  point[index] = static_cast<int32_t>(std::stoul(substr));

  for (int i = 0; i < DIMENSION - 1; i++) {
    if (i == 8 || i == 9) {
      point[i] -= 20110000;
    }
  }

  return point;
}

void flush_vector_to_file(std::vector<TimeStamp> vect, std::string filename){
    std::ofstream outFile(filename);
    for (const auto &e : vect) outFile << std::to_string(e) << "\n";
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

void github()
{
	RTree tree;
	Visitor x;

    std::ifstream infile(GITHUB_DATA_ADDR);

    TimeStamp start, diff;

    diff = 0;
    uint64_t n_points = 0;
    std::string line;
    std::vector<TimeStamp> insertion_latency_vect;

    /**
     * Insertion
     */
    int skip_lines = 0;

    while (std::getline(infile, line))
    {
        if (skip_lines < SKIP_SIZE) {
            skip_lines += 1;
            continue;
        }

		std::vector<int> coordinates = parse_line_github(line);
        coordinates.insert(coordinates.begin(), n_points);

        start = GetTimestamp();
        tree.Insert(dummy, bounds(coordinates));
        TimeStamp latency = GetTimestamp() - start;
        diff += latency;

		if (n_points % (GITHUB_SIZE / 50) == 0)
			std::cout << "finished: " << n_points << std::endl;

        if (n_points > GITHUB_SIZE - points_to_insert)
            insertion_latency_vect.push_back(latency + SERVER_TO_SERVER_IN_NS);

        if (n_points == GITHUB_SIZE)
            break;

        if (n_points == 0 || n_points == points_to_insert / 2) {
            for (int j = 0; j < DIMENSION; j++) {
                std::cout << coordinates[j] << " ";
            }
            std::cout << std::endl;
        }

        n_points += 1;

    }
    std::cout << "Done! " << "Insertion Latency per point: " << (float) diff / n_points << std::endl;
    flush_vector_to_file(insertion_latency_vect, results_folder_addr + "/rstar-tree/github_insert");

    /**
     * Point Lookup
     */

    std::vector<int> max_values = {7451541, 737170, 262926, 354850, 379379, 3097263, 703341, 8745, 20201206, 20201206};
    std::vector<int> min_values = {1, 1, 0, 0, 0, 0, 0, 0, 20110211, 20110211};
    std::vector<TimeStamp> lookup_latency_vect;
    diff = 0; 
    for (int i = 0; i < points_to_lookup; i ++) 
    {
        std::vector<int> start_range(DIMENSION, 0);
        std::vector<int> end_range(DIMENSION, 0);

        start_range[0] = i;
        end_range[0] = i;


        for (int j = 0; j < DIMENSION - 1; j++) {
            start_range[j + 1] = (uint32_t) min_values[j];
            end_range[j + 1] = (uint32_t) max_values[j];
        }

        for (dimension_t j = 1; j < DIMENSION; j++){
            if (j >= 9) {
                start_range[j] -= 20110000;
                end_range[j] -= 20110000;
            }
        }

        if (i == 0) {
            for (int j = 0; j < DIMENSION; j++) {
                std::cout << start_range[j] << " ";
            }
            std::cout << std::endl;
            for (int j = 0; j < DIMENSION; j++) {
                std::cout << end_range[j] << " ";
            }
            std::cout << std::endl;
        }

        BoundingBox bound = bounds_range(start_range, end_range);
        start = GetTimestamp();
        x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;

        if (leaf_list.size() > 1) {
            std::cerr << i << "wrong points!" << leaf_list.size() << std::endl;
            exit(-1);
        }
        leaf_list.clear();

        if (i > points_for_warmup && i <= points_to_lookup)
            lookup_latency_vect.push_back(temp_diff + SERVER_TO_SERVER_IN_NS);
    }
    std::cout << "Done! " << "Lookup Latency per point: " << (float) diff / points_to_lookup << std::endl;
    flush_vector_to_file(lookup_latency_vect, results_folder_addr + "/rstar-tree/github_lookup");

    /**
     * Range Search
     */

    std::ifstream file(GITHUB_QUERY_ADDR);
    std::ofstream outfile(results_folder_addr + "/rstar-tree/github_query");

    for (int i = 0; i < 1000; i ++) 
    {
        std::vector<int> start_range(DIMENSION - 1, 0);
        std::vector<int> end_range(DIMENSION - 1, 0);

        for (int j = 0; j < DIMENSION - 1; j++) {
            start_range[j] = (uint32_t) min_values[j];
            end_range[j] = (uint32_t) max_values[j];
        }

        std::string line;
        std::getline(file, line);

        std::stringstream ss(line);

        while (ss.good()) {

            std::string index_str;
            std::getline(ss, index_str, ',');

            std::string start_range_str;
            std::getline(ss, start_range_str, ',');
            std::string end_range_str;
            std::getline(ss, end_range_str, ',');

            // cout << start_range_str << " " << end_range_str << endl;
            int index = std::stoul(index_str);
            if (index > 10)
                index -= 3;

            if (start_range_str != "-1") {
                start_range[static_cast<int32_t>(index)] = static_cast<int32_t>(std::stoul(start_range_str));
            }
            if (end_range_str != "-1") {
                end_range[static_cast<int32_t>(index)] = static_cast<int32_t>(std::stoul(end_range_str));
            }
        }

        for (dimension_t j = 8; j < DIMENSION - 1; j++){
            start_range[j] -= 20110000;
            end_range[j] -= 20110000;
        }

        start_range.insert(start_range.begin(), 0);
        end_range.insert(end_range.begin(), GITHUB_SIZE);

        if (i == 0) {
            for (int j = 0; j < DIMENSION; j++) {
                std::cout << start_range[j] << " ";
            }
            std::cout << std::endl;
            for (int j = 0; j < DIMENSION; j++) {
                std::cout << end_range[j] << " ";
            }
            std::cout << std::endl;
        }

        BoundingBox bound = bounds_range(start_range, end_range);
        start = GetTimestamp();
        x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
        TimeStamp temp_diff =  GetTimestamp() - start; 

        outfile << "Query " << i << " end to end latency (ms): " << temp_diff / 1000 << ", found points count: " << leaf_list.size() << std::endl;
        leaf_list.clear();
    }

    return;
}

int main(){
    std::cout << "github......" << std::endl;
    github();
}