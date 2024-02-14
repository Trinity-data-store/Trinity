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

#define GITHUB_DIMENSION 10 + 1
#define GITHUB_SIZE 200000000 // 200M
#define SKIP_SIZE 700000000 - GITHUB_SIZE
#define SERVER_TO_SERVER_IN_NS 92

int points_to_insert = 30000;
int points_to_lookup = 30000;

using namespace improbable::phtree;
typedef unsigned long long int TimeStamp;

TimeStamp GetTimestamp()
{
    struct timeval now;
    gettimeofday(&now, nullptr);

    return now.tv_usec + (TimeStamp)now.tv_sec * 1000000;
}

// Parse one line from TPC-H file.
std::vector<int32_t>
parse_line_github(std::string line)
{

    std::vector<int32_t> point(GITHUB_DIMENSION - 1, 0);
    int index = -1;
    bool primary_key = true;
    std::string delim = ",";
    auto start = 0U;
    auto end = line.find(delim);
    // int real_index = -1;
    // [id, events_count, authors_count, forks, stars, issues, pushes, pulls,
    // downloads, adds, dels, add_del_ratio, start_date, end_date]
    while (end != std::string::npos)
    {
        std::string substr = line.substr(start, end - start);
        start = end + 1;
        end = line.find(delim, start);

        if (primary_key)
        {
            primary_key = false;
            continue;
        }
        index++;
        point[index] = static_cast<int32_t>(std::stoul(substr));
    }
    index++;
    std::string substr = line.substr(start, end - start);
    point[index] = static_cast<int32_t>(std::stoul(substr));

    for (int i = 0; i < GITHUB_DIMENSION - 1; i++)
    {
        if (i == 8 || i == 9)
        {
            point[i] -= 20110000;
        }
    }

    return point;
}

std::vector<PhPoint<GITHUB_DIMENSION>> key_list;
PhPoint<GITHUB_DIMENSION> lookup_pt;
int found_cnt = 0;

struct Counter_github
{
    void operator()(PhPoint<GITHUB_DIMENSION> key, uint32_t t)
    {
        ++n_;
        key_list.push_back(key);
    }
    size_t n_ = 0;
};

struct Counter_github_lookup
{
    void operator()(PhPoint<GITHUB_DIMENSION> key, uint32_t t)
    {
        lookup_pt = key;
        found_cnt += 1;
    }
};

void flush_vector_to_file(std::vector<TimeStamp> vect, std::string filename)
{
    std::ofstream outFile(filename);
    for (const auto &e : vect)
        outFile << std::to_string(e) << "\n";
}

void github()
{

    std::ifstream infile(GITHUB_DATA_ADDR);

    PhTree<GITHUB_DIMENSION, bool> tree;
    int n_points = 0;
    bool dummy = true;
    TimeStamp diff, start;
    diff = 0;

    std::string line;
    std::vector<PhPoint<GITHUB_DIMENSION>> point_vect;
    std::vector<TimeStamp> insertion_latency_vect;

    /**
     * Insertion
     */

    int skip_lines = 0;
    TimeStamp start_end_to_end = GetTimestamp();

    while (std::getline(infile, line))
    {
        if (skip_lines < SKIP_SIZE)
        {
            skip_lines += 1;
            continue;
        }

        PhPoint<GITHUB_DIMENSION> p1;
        std::vector<int32_t> vect = parse_line_github(line);
        p1[0] = n_points; // primary key
        for (int i = 0; i < GITHUB_DIMENSION - 1; i++)
        {
            p1[i + 1] = (uint32_t)vect[i]; // Make room for primary key
        }

        if (n_points == 0)
        {
            for (int i = 0; i < GITHUB_DIMENSION; i++)
            {
                std::cout << p1[i] << " ";
            }
            std::cout << std::endl;
        }

        start = GetTimestamp();
        tree.insert(p1, dummy);
        TimeStamp latency = GetTimestamp() - start;
        diff += latency;
        n_points++;

        if (n_points > GITHUB_SIZE - points_to_insert)
            insertion_latency_vect.push_back(latency + SERVER_TO_SERVER_IN_NS);

        if (n_points == GITHUB_SIZE)
            break;

        if (n_points % (GITHUB_SIZE / 100) == 0)
            std::cout << "n_points: " << n_points << std::endl;
    }

    std::cout << "Done! "
              << "Insertion Latency per point: " << (float)(GetTimestamp() - start_end_to_end) / n_points << std::endl;
    flush_vector_to_file(insertion_latency_vect, results_folder_addr + "/ph-tree/github_insert");

    /**
     * Point Lookup
     */

    start_end_to_end = GetTimestamp();
    std::vector<int32_t> max_values = {7451541, 737170, 262926, 354850, 379379, 3097263, 703341, 8745, 20201206 - 20110000, 20201206 - 20110000};
    std::vector<int32_t> min_values = {1, 1, 0, 0, 0, 0, 0, 0, 20110211 - 20110000, 20110211 - 20110000};

    PhPoint<GITHUB_DIMENSION> lowest;
    PhPoint<GITHUB_DIMENSION> heightest;
    std::vector<TimeStamp> lookup_latency_vect;
    TimeStamp cumulative = 0;

    for (int i = 0; i < GITHUB_DIMENSION - 1; i++)
    { // Make room for primary key
        lowest[i + 1] = (uint32_t)min_values[i];
        heightest[i + 1] = (uint32_t)max_values[i];
    }

    for (int i = 0; i < points_to_lookup; i++)
    {

        PhPoint<GITHUB_DIMENSION> start_range = lowest;
        PhPoint<GITHUB_DIMENSION> end_range = heightest;
        start_range[0] = i;
        end_range[0] = i;

        Counter_github_lookup callback;
        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        TimeStamp temp_diff = GetTimestamp() - start;
        cumulative += temp_diff;
        if (found_cnt == 0 || lookup_pt[0] != i)
        {
            std::cout << "wrong number of points found! found_cnt: " << found_cnt << std::endl;
            for (const auto i : lookup_pt)
                std::cout << i << " " << std::endl;
            exit(-1);
        }
        if (i == 0)
        {
            for (int j = 0; j < GITHUB_DIMENSION; j++)
            {
                std::cout << lookup_pt[j] << " ";
            }
            std::cout << std::endl;
        }

        found_cnt = 0;
        if (i <= points_to_lookup)
            lookup_latency_vect.push_back(temp_diff + SERVER_TO_SERVER_IN_NS);
    }
    std::cout << "Done! "
              << "Lookup Latency per point: " << (float)(GetTimestamp() - start_end_to_end) / points_to_lookup << std::endl;
    flush_vector_to_file(lookup_latency_vect, results_folder_addr + "/ph-tree/github_lookup");

    /**
     * Range Search
     */

    max_values = {7451541, 737170, 262926, 354850, 379379, 3097263, 703341, 8745, 20201206, 20201206};
    min_values = {1, 1, 0, 0, 0, 0, 0, 0, 20110211, 20110211};

    std::ifstream file(GITHUB_QUERY_ADDR);
    std::ofstream outfile(results_folder_addr + "/ph-tree/github_query");

    for (int i = 0; i < 1000; i++)
    {

        PhPoint<GITHUB_DIMENSION> start_range;
        PhPoint<GITHUB_DIMENSION> end_range;
        start_range[0] = 0;
        end_range[0] = GITHUB_SIZE;

        for (int i = 0; i < GITHUB_DIMENSION - 1; i++)
        { // Make room for primary key
            start_range[i + 1] = (uint32_t)min_values[i];
            end_range[i + 1] = (uint32_t)max_values[i];
        }

        std::string line;
        std::getline(file, line);

        std::stringstream ss(line);

        while (ss.good())
        {

            std::string index_str;
            std::getline(ss, index_str, ',');

            std::string start_range_str;
            std::getline(ss, start_range_str, ',');
            std::string end_range_str;
            std::getline(ss, end_range_str, ',');

            int index = std::stoul(index_str);
            if (index > 10)
                index -= 3;

            if (start_range_str != "-1")
            {
                start_range[static_cast<int32_t>(index) + 1] = static_cast<int32_t>(std::stoul(start_range_str));
            }
            if (end_range_str != "-1")
            {
                end_range[static_cast<int32_t>(index) + 1] = static_cast<int32_t>(std::stoul(end_range_str));
            }
        }

        for (dimension_t i = 1; i < GITHUB_DIMENSION; i++)
        {
            if (i >= 9)
            {
                start_range[i] -= 20110000;
                end_range[i] -= 20110000;
            }
        }

        if (i == 0)
        {
            for (int j = 0; j < GITHUB_DIMENSION; j++)
            {
                std::cout << start_range[j] << " ";
            }
            std::cout << std::endl;
            for (int j = 0; j < GITHUB_DIMENSION; j++)
            {
                std::cout << end_range[j] << " ";
            }
            std::cout << std::endl;
        }

        Counter_github callback;
        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        TimeStamp temp_diff = GetTimestamp() - start;
        outfile << "Query " << i << " end to end latency (ms): " << temp_diff / 1000 << ", found points count: " << key_list.size() << std::endl;
        key_list.clear();
    }
}

int main(int argc, char *argv[])
{

    std::cout << "github......" << std::endl;
    github();
}