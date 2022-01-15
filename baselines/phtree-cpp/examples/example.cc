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
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <signal.h>

using namespace improbable::phtree;
typedef unsigned long long int TimeStamp;

TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

std::vector<PhPoint<4> *> key_list;
// Callback for counting entries
struct Counter {
    void operator()(PhPoint<4> key, uint32_t t) {
        ++n_;
        key_list.push_back(&key);
    }
    size_t n_ = 0;
};

std::vector<PhPoint<7> *> key_list_filesys;

struct Counter_filesys {
    void operator()(PhPoint<7> key, uint32_t t) {
        ++n_;
        key_list_filesys.push_back(&key);
    }
    size_t n_ = 0;
};

std::vector<PhPoint<9> *> key_list_tpch;

struct Counter_tpch {
    void operator()(PhPoint<9> key, uint32_t t) {
        ++n_;
        key_list_tpch.push_back(&key);
    }
    size_t n_ = 0;
};

void filesys() {

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../../../data/fs/fs_dataset.txt", "r");

    // If the file cannot be open
    if (fp == nullptr)
    {
        fprintf(stderr, "file not found\n");
        exit(EXIT_FAILURE);
    }

    PhTree<7, uint64_t> tree;
    int n_points = 0;
    TimeStamp diff, start;
    diff = 0;
    int64_t max[7] = {0};
    int64_t min[7] = {0};

    /**
     * Insertion
     */

    while ((read = getline(&line, &len, fp)) != -1)
    {

        char *token = strtok(line, " "); // id
        char *ptr;
        PhPoint<7> p1;
        for (uint8_t i = 1; i <= 2; i ++){
            token = strtok(nullptr, " ");
        }
        for (dimension_t i = 0; i < 7; i++){
            token = strtok(nullptr, " ");
            p1[i] = strtoul(token, &ptr, 10);
        }

        start = GetTimestamp();
        tree.emplace(p1, n_points);
        diff += GetTimestamp() - start;
        if (n_points % 1000000 == 0)
            std::cout << "n_points: " << n_points << std::endl;

        for (dimension_t i = 0; i < 7; i++){
            if (n_points == 0){
                max[i] = p1[i];
                min[i] = p1[i];
            }
            else {
                if (p1[i] > max[i]){
                    max[i] = p1[i];
                }
                if (p1[i] < min[i]){
                    min[i] = p1[i];
                }
            }            
        }
        n_points ++;
    }    
    std::cout << "Done! " << "Insertion Latency per point" << (float) diff / n_points << std::endl;

    /**
     * Benchmark range search given a query selectivity (1000-2000), given a query
     */

    line = nullptr;
    len = 0;
    fp = fopen("../../../queries/fs/fs_range_queries.csv", "r");
    int count = 0;
    diff = 0;
    read = getline(&line, &len, fp);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        PhPoint<7> start_range;
        PhPoint<7> end_range;

        char *ptr;
        char *token = strtok(line, ","); // id

        for (dimension_t i = 0; i < 6; i++){
            token = strtok(nullptr, ","); // id
            start_range[i] = strtoul(token, &ptr, 10);
            token = strtok(nullptr, ",");
            end_range[i] = strtoul(token, &ptr, 10);
        }
        start_range[6] = min[6];
        end_range[6] = max[6];

        Counter_filesys callback;
        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;

        count ++;   
        key_list_tpch.clear();
    }
    std::cout << "average query latency: " << (float) diff / count << std::endl;    
}

void osm() {

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../../../data/osm/osm_dataset.csv", "r");
    PhTree<4, uint32_t> tree;
    int n_points = 0;
    TimeStamp diff, start;
    diff = 0;
    int64_t max[4] = {0};
    int64_t min[4] = {0};
    read = getline(&line, &len, fp);
    std::vector <PhPoint<4>> point_vect;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *token = strtok(line, ","); // id
        char *ptr;
        PhPoint<4> p1;
        for (dimension_t i = 0; i < 4; i++){
            token = strtok(nullptr, ",");
            p1[i] = strtoul(token, &ptr, 10);
        }

        start = GetTimestamp();
        tree.emplace(p1, n_points);
        diff += GetTimestamp() - start;
        if (n_points % 3000000 == 0)
            std::cout << "n_points: " << n_points << std::endl;

        for (dimension_t i = 0; i < 4; i++){
            if (n_points == 0){
                max[i] = p1[i];
                min[i] = p1[i];
            }
            else {
                if (p1[i] > max[i]){
                    max[i] = p1[i];
                }
                if (p1[i] < min[i]){
                    min[i] = p1[i];
                }
            }            
        }
        n_points ++;
    }    
    std::cout << "Done! " << "Insertion Latency per point" << (float) diff / n_points << std::endl;

    line = nullptr;
    len = 0;
    fp = fopen("../../../queries/osm/osm_range_queries.csv", "r");
    int count = 0;
    diff = 0;
    read = getline(&line, &len, fp);
    
    while ((read = getline(&line, &len, fp)) != -1)
    {
        PhPoint<4> start_range;
        PhPoint<4> end_range;

        char *ptr;
        char *token = strtok(line, ","); // id

        for (dimension_t i = 0; i < 4; i++){
            token = strtok(nullptr, ","); // id
            start_range[i] = strtoul(token, &ptr, 10);
            token = strtok(nullptr, ",");
            end_range[i] = strtoul(token, &ptr, 10);
        }
        count ++;   

        Counter callback;
        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;
        key_list.clear();
    }
    std::cout << "average query latency: " << (float) diff / count << std::endl;    
}

void tpch() {

    std::ifstream infile("../../../data/tpc-h/tpch_dataset.csv");
    
    PhTree<9, uint32_t> tree;
    int n_points = 0;
    TimeStamp diff, start;
    diff = 0;
    int64_t max[9] = {0};
    int64_t min[9] = {0};
    std::string line;
    std::getline(infile, line);
    std::vector<PhPoint<9>> point_vect;

    /**
     * Insertion
     */
    
    while (std::getline(infile, line))
    {
        PhPoint<9> p1;

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

            p1[leaf_point_index] = num;
            leaf_point_index++;
        }
        start = GetTimestamp();
        tree.emplace(p1, n_points);
        diff += GetTimestamp() - start;
        if (n_points % 1000000 == 0)
            std::cout << "n_points: " << n_points << std::endl;

        for (dimension_t i = 0; i < 9; i++){
            if (n_points == 0){
                max[i] = p1[i];
                min[i] = p1[i];
            }
            else {
                if (p1[i] > max[i]){
                    max[i] = p1[i];
                }
                if (p1[i] < min[i]){
                    min[i] = p1[i];
                }
            }            
        }
        n_points ++;

    }    
    std::cout << "Done! " << "Insertion Latency per point" << (float) diff / n_points << std::endl;

    /**
     * Benchmark range search given a query selectivity (1000-2000), given a query
     */

    char *line_query = nullptr;
    size_t len = 0;
    FILE *fp = fopen("../../../queries/tpch/tpch_range_queries.csv", "r");
    int count = 0;
    diff = 0;
    ssize_t read = getline(&line_query, &len, fp);

    while ((read = getline(&line_query, &len, fp)) != -1)
    {
        PhPoint<9> start_range;
        PhPoint<9> end_range;

        char *ptr;
        char *token = strtok(line_query, ","); // id

        for (uint8_t j = 0; j < 9; j++){
            token = strtok(nullptr, ","); 
            start_range[j] = strtoul(token, &ptr, 10);
            token = strtok(nullptr, ",");
            end_range[j] = strtoul(token, &ptr, 10);
        }
        Counter_tpch callback;
        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        diff += GetTimestamp() - start;
        key_list_tpch.clear();
        count ++;
    }
    std::cout << "average query latency: " << (float) diff / count << std::endl;    
}


int main(int argc, char *argv[]){

    if (argc != 2){
        std::cerr << "wrong number of input!" << std::endl;
        exit(0);
    }
    
    int dataset_index = atoi(argv[1]);

    if (dataset_index == 0){
        std::cout << "file system......" << std::endl;
        filesys();
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