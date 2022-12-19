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
#include "../../librpc/src/TrinityParseFIle.h"

#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>

#define NYC_SIZE 200000000 // 200M

#define DIMENSION 15 + 1
#define LATENCY_BENCH

int points_to_insert = 30000;
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

void nyc()
{
	RTree tree;
	Visitor x;

    std::ifstream infile("/mntData2/nyc_taxi/nyc_taxi_processed_675.csv");

    TimeStamp start, diff;

    diff = 0;
    uint64_t n_points = 0;
    std::string line;
    std::vector<TimeStamp> insertion_latency_vect;

    /**
     * Insertion
     */

    while (std::getline(infile, line))
    {
		std::vector<int> coordinates = parse_line_nyc(line);
        coordinates.insert(coordinates.begin(), n_points);

        start = GetTimestamp();
        tree.Insert(dummy, bounds(coordinates));
        TimeStamp latency = GetTimestamp() - start;
        diff += latency;

        // coordinates.resize(DIMENSION);

		if (n_points % (NYC_SIZE / 50) == 0)
			std::cout << "finished: " << n_points << std::endl;
    
        #ifdef LATENCY_BENCH
        if (n_points == points_to_insert)
            break;    
        #else
        if (n_points == NYC_SIZE)
            break;
        #endif

        if (n_points > points_for_warmup && n_points <= points_to_insert)
            insertion_latency_vect.push_back(latency);

        if (n_points == 0 || n_points == points_to_insert / 2) {
            for (int j = 0; j < DIMENSION; j++) {
                std::cout << coordinates[j] << " ";
            }
            std::cout << std::endl;
        }

        n_points += 1;

    }
    std::cout << "Done! " << "Insertion Latency per point: " << (float) diff / n_points << std::endl;
    flush_vector_to_file(insertion_latency_vect, "./rstar_nyc_insert");
    // flush_vector_to_file(insertion_latency_vect, "/proj/trinity-PG0/Trinity/results/latency_cdf/rstar_nyc_insert");
    exit(0);

    /**
     * Range Search
     */

    std::vector<int> max_values = {20160630, 20221220, 899, 898, 899, 898, 255, 198623000, 21474808, 1000, 1312,  3950589, 21474836, 138, 21474830};
    std::vector<int> min_values = {20090101, 19700101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    #ifndef LATENCY_BENCH
    char *infile_address = (char *)"/proj/trinity-PG0/Trinity/queries/nyc/nyc_query_new_converted";
    char *outfile_address = (char *)"/proj/trinity-PG0/Trinity/results/nyc_rstar";


    std::ifstream file(infile_address);
    std::ofstream outfile(outfile_address);


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
        // cout << line << endl;
        // Example: 0,-1,24,2,5,7,4,19943347,19950101
        while (ss.good()) {

            std::string index_str;
            std::getline(ss, index_str, ',');

            std::string start_range_str;
            std::getline(ss, start_range_str, ',');
            std::string end_range_str;
            std::getline(ss, end_range_str, ',');

            // cout << start_range_str << " " << end_range_str << endl;
            int index = std::stoul(index_str);
            if (start_range_str != "-1") {
                if (index >= 2 && index <= 5) {
                    float num_float = std::stof(start_range_str);
                    start_range[static_cast<int32_t>(index)] = static_cast<int32_t>(num_float * 10);                    
                }
                else 
                    start_range[static_cast<int32_t>(index)] = static_cast<int32_t>(std::stoul(start_range_str));
            }
            if (end_range_str != "-1") {
                if (index >= 2 && index <= 5) {
                    float num_float = std::stof(end_range_str);
                    end_range[static_cast<int32_t>(index)] = static_cast<int32_t>(num_float * 10);                    
                }
                else 
                    end_range[static_cast<int32_t>(index)] = static_cast<int32_t>(std::stoul(end_range_str));
            }
        }
        
        start_range[0] -= 20090000;
        start_range[1] -= 19700000;
        end_range[0] -= 20090000;
        end_range[1] -= 19700000;

        start_range.insert(start_range.begin(), 0);
        end_range.insert(end_range.begin(), NYC_SIZE);

        // start_range.resize(DIMENSION);
        // end_range.resize(DIMENSION);
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
    #endif

    /**
     * Point Lookup
     */
    std::vector<TimeStamp> lookup_latency_vect;
    diff = 0; 

    for (int i = 0; i < points_to_insert; i ++) 
    {
        std::vector<int> start_range(DIMENSION, 0);
        std::vector<int> end_range(DIMENSION, 0);

        start_range[0] = i;
        end_range[0] = i;

        for (int j = 0; j < DIMENSION - 1; j++) {
            start_range[j + 1] = (uint32_t) min_values[j];
            end_range[j + 1] = (uint32_t) max_values[j];
        }

        start_range[0 + 1] -= 20090000;
        start_range[1 + 1] -= 19700000;
        end_range[0 + 1] -= 20090000;
        end_range[1 + 1] -= 19700000;

        BoundingBox bound = bounds_range(start_range, end_range);
        start = GetTimestamp();
        x = tree.Query(RTree::AcceptEnclosing(bound), Visitor());
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;

        if (leaf_list.size() > 1) {
            std::cerr << "wrong points! " << leaf_list.size() << std::endl;
            exit(-1);
        }

        leaf_list.clear();
        if (i > points_for_warmup && i <= points_to_insert)
            lookup_latency_vect.push_back(temp_diff);
    }
    std::cout << "Done! " << "Lookup Latency per point: " << (float) diff / points_to_insert << std::endl;
    flush_vector_to_file(lookup_latency_vect, "/proj/trinity-PG0/Trinity/results/latency_cdf/rstar_nyc_lookup");
    return;
}

int main(){
    std::cout << "nyc......" << std::endl;
    nyc();
}