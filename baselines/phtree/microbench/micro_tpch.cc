/*
 * Copyright 2020 Improbable Worlds Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../phtree/phtree.h"
#include "./micro_common.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <cmath>
#include <vector>

#define TPCH_DIMENSION 9 + 1
#define TPCH_SIZE 250000000 // 250M
#define SERVER_TO_SERVER_IN_NS 92

int points_to_insert = 30000;
int points_to_lookup = 30000;
int points_for_warmup = points_to_insert / 5;

using namespace improbable::phtree;
typedef unsigned long long int TimeStamp;

TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

std::vector<int32_t>
parse_line_tpch(std::string line)
{

  std::vector<int32_t> point(TPCH_DIMENSION - 1, 0);
  int index = -1;
  bool primary_key = true;
  std::string delim = ",";
  auto start = 0U;
  auto end = line.find(delim);

  // [id, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE,
  // RECEIPTDATE, TOTALPRICE, ORDERDATE]
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

  for (int i = 0; i < TPCH_DIMENSION - 1; i++) {
    if (i >= 4 && i != 7) {
      point[i] -= 19000000;
    }
  }

  return point;
}

std::vector<PhPoint<TPCH_DIMENSION>> key_list;
PhPoint<TPCH_DIMENSION> lookup_pt;
int found_cnt = 0;

struct Counter_tpch {
    void operator()(PhPoint<TPCH_DIMENSION> key, uint32_t t) {
        ++n_;
        key_list.push_back(key);
    }
    size_t n_ = 0;
};

struct Counter_tpch_lookup {
    void operator()(PhPoint<TPCH_DIMENSION> key, uint32_t t) {
        lookup_pt = key;
        found_cnt += 1;
    }
};

void flush_vector_to_file(std::vector<TimeStamp> vect, std::string filename){
    std::ofstream outFile(filename);
    for (const auto &e : vect) outFile << std::to_string(e) << "\n";
}

void tpch() {

    std::ifstream infile(TPCH_DATA_ADDR);
    
    PhTree<TPCH_DIMENSION, bool> tree;
    int n_points = 0;
    bool dummy = true;
    TimeStamp diff, start;
    diff = 0;

    std::string line;
    std::vector<PhPoint<TPCH_DIMENSION>> point_vect;
    std::vector<TimeStamp> insertion_latency_vect;

    /**
     * Insertion
     */
    
    TimeStamp start_end_to_end = GetTimestamp();

    while (std::getline(infile, line))
    {
        PhPoint<TPCH_DIMENSION> p1;
        std::vector<int32_t> vect = parse_line_tpch(line);
        p1[0] = n_points; // primary key

        for (int i = 0; i < TPCH_DIMENSION - 1; i++) {
            p1[i + 1] = (uint32_t) vect[i];
        }

        if (n_points == 0 || n_points == points_to_insert / 2) {
            for (int i = 0; i < TPCH_DIMENSION; i++) {
                std::cout << p1[i] << " ";
            }
            std::cout << std::endl;
        }

        start = GetTimestamp();
        tree.insert(p1, dummy);
        TimeStamp latency =  GetTimestamp() - start;
        diff += latency;
        n_points ++;

        if (n_points > TPCH_SIZE - points_to_insert)
            insertion_latency_vect.push_back(latency + SERVER_TO_SERVER_IN_NS);

        if (n_points == TPCH_SIZE)
            break;
        
        if (n_points % (TPCH_SIZE / 100) == 0)
            std::cout << "n_points: " << n_points << std::endl;
    }    

    std::cout << "Done! " << "Insertion Latency per point: " << (float) (GetTimestamp() - start_end_to_end) / n_points << std::endl;
    flush_vector_to_file(insertion_latency_vect, results_folder_addr + "/ph-tree/tpch_insert");

    /**
     * Point Lookup
     */
    
    start_end_to_end = GetTimestamp();
    std::vector<int32_t> max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 58063825, 19980802};
    std::vector<int32_t> min_values = {1, 90000, 0, 0, 19920102, 19920131, 19920103, 81300, 19920101};

    for (dimension_t i = 0; i < TPCH_DIMENSION; i++){
        if (i >= 4 && i != 7) {
            max_values[i] -= 19000000;
            min_values[i] -= 19000000;
        }
    }

    PhPoint<TPCH_DIMENSION> lowest;
    PhPoint<TPCH_DIMENSION> heightest;
    std::vector<TimeStamp> lookup_latency_vect;
    TimeStamp cumulative = 0;

    for (int i = 0; i < TPCH_DIMENSION - 1; i++) { // Make room for primary key
        lowest[i + 1] = (uint32_t) min_values[i];
        heightest[i + 1] = (uint32_t) max_values[i];
    }

    for (int i = 0; i < points_to_lookup; i ++) {

        PhPoint<TPCH_DIMENSION> start_range = lowest;
        PhPoint<TPCH_DIMENSION> end_range = heightest;
        start_range[0] = i;
        end_range[0] = i;

        Counter_tpch_lookup callback;

        cnt = 0;
        cnt_traverse_node = 0;
        for_cnt = 0;

        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        TimeStamp temp_diff = GetTimestamp() - start;
        cumulative += temp_diff;
    
        if (found_cnt == 0 || lookup_pt[0] != i) {
            std::cout << "wrong number of points found! found_cnt: " << found_cnt << "i: " << i << std::endl;
            for (const auto i : lookup_pt) std::cout << i << " " << std::endl;
            exit(-1);
        }
        if (i == 0 || i == points_to_lookup / 2) {
            for (int j = 0; j < TPCH_DIMENSION; j++) {
                std::cout << lookup_pt[j] << " ";
            }
            std::cout << std::endl;
        }

        found_cnt = 0;
        if (i > points_for_warmup && i <= points_to_lookup)
            lookup_latency_vect.push_back(temp_diff + SERVER_TO_SERVER_IN_NS);

    }
    std::cout << "Done! " << "Lookup Latency per point: " << (float) (GetTimestamp() - start_end_to_end) / points_to_lookup << std::endl;
    flush_vector_to_file(lookup_latency_vect, results_folder_addr + "/ph-tree/tpch_lookup");
    
    /**
     * Range Search
     */

    // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
    max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 58063825, 19980802};
    min_values = {1, 90000, 0, 0, 19920102, 19920131, 19920103, 81300, 19920101};

    std::ifstream file(TPCH_QUERY_ADDR);
    std::ofstream outfile(results_folder_addr + "/ph-tree/tpch_query");

    for (int i = 0; i < 1000; i ++) {

        PhPoint<TPCH_DIMENSION> start_range;
        PhPoint<TPCH_DIMENSION> end_range;
        start_range[0] = 0;
        end_range[0] = TPCH_SIZE;

        for (int i = 0; i < TPCH_DIMENSION - 1; i++) {
            start_range[i + 1] = (uint32_t) min_values[i];
            end_range[i + 1] = (uint32_t) max_values[i];
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

            if (start_range_str != "-1") {
                start_range[static_cast<int32_t>(std::stoul(index_str)) + 1] = static_cast<int32_t>(std::stoul(start_range_str));
            }
            if (end_range_str != "-1") {
                end_range[static_cast<int32_t>(std::stoul(index_str)) + 1] = static_cast<int32_t>(std::stoul(end_range_str));
            }
        }

        for (dimension_t i = 0; i < TPCH_DIMENSION; i++){
            if (i >= 4 && i != 7 && i != TPCH_DIMENSION - 1) {
                start_range[i + 1] -= 19000000;
                end_range[i + 1] -= 19000000;
            }
        }

        if (i == 0) {
            for (int j = 0; j < TPCH_DIMENSION; j++) {
                std::cout << start_range[j] << " ";
            }
            std::cout << std::endl;
            for (int j = 0; j < TPCH_DIMENSION; j++) {
                std::cout << end_range[j] << " ";
            }
            std::cout << std::endl;
        }

        Counter_tpch callback;
        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        TimeStamp temp_diff = GetTimestamp() - start;

        outfile << "Query " << i << " end to end latency (ms): " << temp_diff / 1000 << ", found points count: " << key_list.size() << std::endl;
        key_list.clear();

    }
}

int main(int argc, char *argv[]){

    std::cout << "tpch......" << std::endl;
    tpch();
}