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

int itr_num = 300;
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


int filesys() {

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("/home/ziming/md-trie/data/fs/fs_dataset.txt", "r");
    PhTree<7, uint64_t> tree;
    int n_points = 0;
    TimeStamp diff, start;
    diff = 0;
    int64_t max[7] = {0};
    int64_t min[7] = {0};

    // std::vector<PhPoint<7>> point_vect;

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
        // point_vect.push_back(p1);

    }    
    std::cout << "Done! " << "Insertion Latency per point" << (float) diff / n_points << std::endl;
    exit(0);

/*
    line = nullptr;
    len = 0;   
    diff = 0;
    n_points = 0;
    FILE *fp2 = fopen("/home/ziming/md-trie/libmdtrie/bench/data/sample_shuf.txt", "r");
    point_vect.clear();
    // std::vector <PhPoint<7>> point_vect;

    while ((read = getline(&line, &len, fp2)) != -1)
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
        tree.find(p1);
        diff += GetTimestamp() - start;
        if (n_points % 1000000 == 0)
            std::cout << "n_points: " << n_points << std::endl;

        n_points ++;
        point_vect.push_back(p1);

    }    
    std::cout << "Done! " << "Find Latency per point" << (float) diff / n_points << std::endl;

    PhPoint<7> start_range;
    PhPoint<7> end_range;
    for (uint8_t j = 0; j < 7; j++){
        start_range[j] = min[j];
        end_range[j] = max[j] + 1;
    }
    Counter_filesys callback;
    start = GetTimestamp();
    tree.for_each({start_range, end_range}, callback);
    diff = GetTimestamp() - start;

    std::cout << "found one: " << callback.n_ << "," << diff << "," << " n_points: "<< n_points << std::endl;
    std::cout << "All points in range:" << start_range << "/" << end_range << std::endl << std::endl;



    line = nullptr;
    len = 0;
    fp = fopen("/home/ziming/phtree-cpp/build/filesys_phtree_queries_1000.csv", "r");
    int count = 0;
    diff = 0;
    std::ofstream file_range_search("filesys_phtree_queries_1000_7_redo.csv");

    while ((read = getline(&line, &len, fp)) != -1)
    {
        PhPoint<7> start_range;
        PhPoint<7> end_range;

        char *ptr;
        char *token = strtok(line, ","); // id
        token = strtok(nullptr, ",");
        token = strtok(nullptr, ",");

        for (dimension_t i = 0; i < 6; i++){
            token = strtok(nullptr, ","); // id
            start_range[i] = strtoul(token, &ptr, 10);
            token = strtok(nullptr, ",");
            end_range[i] = strtoul(token, &ptr, 10);
        }
        start_range[6] = min[6];
        end_range[6] = max[6];

        int present_pt_count = 0;
        for (unsigned int i = 0; i < point_vect.size(); i++){
            bool match = true;
            for (dimension_t j = 0; j < 7; j++){
                if ( point_vect[i][j] < start_range[j] || point_vect[i][j] > end_range[j]){
                    match = false;
                    break;
                }
            }
            if (match){
                present_pt_count ++;
            }
        }   
        std::cout << "present point count: " << present_pt_count << std::endl;

        Counter_filesys callback;
        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;

        count ++;   
        std::cout << "x.count: " << callback.n_ << std::endl; 
        std::cout << "diff: " << temp_diff << std::endl;
        file_range_search << callback.n_ << "," << temp_diff << std::endl; 
        key_list_tpch.clear();
    }
    std::cout << "average query latency: " << (float) diff / count << std::endl;    

    exit(0);
    int itr = 0;
    std::ofstream file("filesys_phtree_queries_1000_dimension7.csv", std::ios_base::app);
    srand(time(NULL));
    max[6] = 4294967295;

    while (itr < itr_num){


        PhPoint<7> start_range;
        PhPoint<7> end_range;
        for (uint8_t j = 0; j < 7; j++){
            start_range[j] = min[j] + rand() % ((uint32_t) (max[j] - min[j]));
            end_range[j] = start_range[j] + (max[j] - start_range[j] + 1) / 5 * (rand() % 5);
        }
        Counter_filesys callback;
        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        diff = GetTimestamp() - start;

        key_list_tpch.clear();
        // if (callback.n_ > 0){
        //     file << callback.n_ << "," << diff << "," << std::endl;
        //     itr ++;
        // }

        if (callback.n_ >= 1000 && callback.n_ <= 2000){

            int present_pt_count = 0;
            for (unsigned int i = 0; i < point_vect.size(); i++)
            {
                bool match = true;
                for (unsigned int j = 0; j < 7; j++){
                    if ( point_vect[i][j] < start_range[j] || point_vect[i][j] > end_range[j]){
                        match = false;
                        break;
                    }
                }
                if (match)
                    present_pt_count ++;                
            }

            if (present_pt_count < 1000 || present_pt_count > 2000)
                continue;

            std::cout << "present point count: " << present_pt_count << " callback.n_: "<< callback.n_ << std::endl;


            file << callback.n_ << "," << diff << "," << "search_range,";
            for (uint8_t j = 0; j < 7; j ++)
            {
                // std::cout << start_range[j] << " " << end_range[j] << " " << std::endl;
                file << start_range[j] << "," << end_range[j] << ",";
            }
            file << std::endl;
            itr ++;
            std::cout << "Found a query!" << std::endl;

        }
    }
*/
    return 0;
}


int osm() {

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
        // point_vect.push_back(p1);
        // for (dimension_t i = 0; i < 4; i++){
        //     std::cout << p1[i] << " ";
        // }
        // exit(0);
    }    
    std::cout << "Done! " << "Insertion Latency per point" << (float) diff / n_points << std::endl;
    std::cout << "n_points: " << n_points << std::endl;

    line = nullptr;
    len = 0;
    fp = fopen("/home/ziming/phtree-cpp/build/osm_phtree_queries_1000.csv", "r");
    int count = 0;
    diff = 0;
    std::ofstream file_range_search("osm_phtree_queries_1000_results.csv");
    while ((read = getline(&line, &len, fp)) != -1)
    {
        PhPoint<4> start_range;
        PhPoint<4> end_range;

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
        int present_pt_count = 0;
        for (unsigned int i = 0; i < point_vect.size(); i++){
            bool match = true;
            for (dimension_t j = 0; j < 4; j++){
                if ( point_vect[i][j] < start_range[j] || point_vect[i][j] > end_range[j]){
                    match = false;
                    break;
                }
            }
            if (match){
                present_pt_count ++;
            }
        }   
        std::cout << "present point count: " << present_pt_count << std::endl;

        Counter callback;
        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        TimeStamp temp_diff =  GetTimestamp() - start; 
        diff += temp_diff;
        key_list.clear();

        std::cout << "callback.n_: " << callback.n_ << std::endl; 
        std::cout << "diff: " << temp_diff << std::endl;
        file_range_search << callback.n_ << "," << temp_diff << std::endl; 
    }
    std::cout << "average query latency: " << (float) diff / count << std::endl;    
    exit(0);

/*
    line = nullptr;
    len = 0;   
    diff = 0;
    n_points = 0;
    FILE *fp2 = fopen("/home/ziming/osm/osm_us_northeast_timestamp.csv", "r");

    while ((read = getline(&line, &len, fp2)) != -1)
    {
        char *token = strtok(line, ","); // id
        char *ptr;
        PhPoint<4> p1;
        for (dimension_t i = 0; i < 4; i++){
            token = strtok(nullptr, ",");
            p1[i] = strtoul(token, &ptr, 10);
        }
        n_points ++;
        start = GetTimestamp();
        tree.find(p1);
        diff += GetTimestamp() - start;
        if (n_points % 3000000 == 0)
            std::cout << "n_points: " << n_points << std::endl;
    }    
    std::cout << "Done! " << "Find Latency per point" << (float) diff / n_points << std::endl;

*/
/*
    PhPoint<4> start_range;
    PhPoint<4> end_range;
    for (uint8_t j = 0; j < 4; j++){
        start_range[j] = 0;
        end_range[j] = max[j] + 1;
    }
    Counter callback;
    start = GetTimestamp();
    tree.for_each({start_range, end_range}, callback);
    diff = GetTimestamp() - start;

    std::cout << "found one: " << callback.n_ << "," << diff << "," << std::endl;
    std::cout << "All points in range:" << start_range << "/" << end_range << std::endl << std::endl;
*/
    int itr = 0;
    std::ofstream file("osm_phtree_queries_1000.csv", std::ios_base::app);
    srand(time(NULL));

    while (itr < itr_num){

        PhPoint<4> start_range;
        PhPoint<4> end_range;
        for (uint8_t j = 0; j < 4; j++){
            start_range[j] = min[j] + rand() % ((uint32_t) (max[j] - min[j]));
            // start_range[j] = min[j] + rand() % ((uint32_t) (max[j] - min[j]));
            end_range[j] = start_range[j] + (max[j] - start_range[j] + 1) / 20 * (rand() % 20);
            // end_range[j] = start_range[j] +  rand() % ((uint32_t) (max[j] - start_range[j]));
        }

        Counter callback;
        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        diff = GetTimestamp() - start;
        key_list_tpch.clear();

        if (callback.n_ >= 1000 && callback.n_ <= 2000){

            int present_pt_count = 0;
            for (unsigned int i = 0; i < point_vect.size(); i++)
            {
                bool match = true;
                for (unsigned int j = 0; j < 4; j++){
                    if ( point_vect[i][j] < start_range[j] || point_vect[i][j] > end_range[j]){
                        match = false;
                        break;
                    }
                }
                if (match)
                    present_pt_count ++;                
            }

            if (present_pt_count < 1000 || present_pt_count > 2000)
                continue;

            std::cout << "present point count: " << present_pt_count << " callback.n_: "<< callback.n_ << std::endl;

            file << callback.n_ << "," << diff << "," << "search_range,";
            for (uint8_t j = 0; j < 4; j ++)
            {
                // std::cout << start_range[j] << " " << end_range[j] << " " << std::endl;
                file << start_range[j] << "," << end_range[j] << ",";
            }
            file << std::endl;
            itr ++;
            std::cout << "Found a query!" << std::endl;

        }
    }

    return 0;

}

int tpch() {

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
        // point_vect.push_back(p1);
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

/*
    n_points = 0;
    for (uint32_t i = 0; i < point_vect.size(); i++){

        PhPoint<9> p1 = point_vect[i];

        start = GetTimestamp();
        tree.find(p1);
        diff += GetTimestamp() - start;
        if (n_points % 1000000 == 0)
            std::cout << "n_points: " << n_points << std::endl;
        n_points ++;
    }    
    std::cout << "Done! " << "Find Latency per point" << (float) diff / n_points << std::endl;


    PhPoint<9> start_range;
    PhPoint<9> end_range;
    for (uint8_t j = 0; j < 9; j++){
        start_range[j] = 0;
        end_range[j] = max[j] + 1;
    }
    Counter_tpch callback;
    start = GetTimestamp();
    tree.for_each({start_range, end_range}, callback);
    diff = GetTimestamp() - start;

    std::cout << "found one: " << callback.n_ << "," << diff << "," << " n_points: "<< n_points << std::endl;
    std::cout << "All points in range:" << start_range << "/" << end_range << std::endl << std::endl;
*/

    int itr = 0;
    std::ofstream file("tpch_phtree_queries_1000.csv", std::ios_base::app);
    srand(time(NULL));

    while (itr < itr_num){

        PhPoint<9> start_range;
        PhPoint<9> end_range;
        for (uint8_t j = 0; j < 9; j++){
            start_range[j] = min[j] + rand() % ((uint32_t) (max[j] - min[j]));
            end_range[j] = start_range[j] + (max[j] - start_range[j] + 1) / 10 * (rand() % 10);
        }
        Counter_tpch callback;
        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        diff = GetTimestamp() - start;
        key_list_tpch.clear();
        // if (callback.n_ > 0){
        //     file << callback.n_ << "," << diff << "," << std::endl;
        //     itr ++;
        // }

        if (callback.n_ >= 1000 && callback.n_ <= 2000){
            
            int present_pt_count = 0;
            for (unsigned int i = 0; i < point_vect.size(); i++)
            {
                bool match = true;
                for (unsigned int j = 0; j < 9; j++){
                    if ( point_vect[i][j] < start_range[j] || point_vect[i][j] > end_range[j]){
                        match = false;
                        break;
                    }
                }
                if (match)
                    present_pt_count ++;                
            }

            if (present_pt_count < 1000 || present_pt_count > 2000)
                continue;

            std::cout << "present point count: " << present_pt_count << " callback.n_: "<< callback.n_ << std::endl;

            file << callback.n_ << "," << diff << "," << "search_range,";
            for (uint8_t j = 0; j < 9; j ++)
            {
                file << start_range[j] << "," << end_range[j] << ",";
            }
            file << std::endl;
            itr ++;
            std::cout << "Found a query!" << std::endl;


        }

    }

    return 0;
}


int main(){

    // itr_num = 1000;

    // std::cout << "********** osm **********" << std::endl;   
    // osm();
    // std::cout << "********** filesys **********" << std::endl;
    // filesys();
    std::cout << "********** tpch **********" << std::endl;
    tpch();
}