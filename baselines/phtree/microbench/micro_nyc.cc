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

#define NYC_DIMENSION 15 + 1
#define NYC_SIZE 200000000 // 200M
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

std::vector<int32_t>
parse_line_nyc(std::string line)
{

    std::vector<int32_t> point(NYC_DIMENSION - 1, 0);
    bool primary_key = true;
    std::string delim = ",";
    auto start = 0U;
    auto end = line.find(delim);
    int real_index = -1;

    // pickup_date, dropoff_date, pickup_longitude, pickup_latitude,
    // dropoff_longitude, dropoff_latitude, passenger_count, trip_distance,
    // fare_amount, extra, mta_tax, tip_amount, tolls_amount,
    // improvement_surcharge, total_amount
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

        real_index++;

        if (real_index >= 2 && real_index <= 5)
        {
            float num_float = std::stof(substr);
            point[real_index] = static_cast<int32_t>(num_float * 10);
        }
        else
        {
            point[real_index] = static_cast<int32_t>(std::stoul(substr));
        }
    }

    real_index++;
    std::string substr = line.substr(start, end - start);
    point[real_index] = static_cast<int32_t>(std::stoul(substr));

    point[0] -= 20090000;
    point[1] -= 19700000;

    return point;
}

std::vector<PhPoint<NYC_DIMENSION>> key_list;
PhPoint<NYC_DIMENSION> lookup_pt;
int found_cnt = 0;

struct Counter
{
    void operator()(PhPoint<NYC_DIMENSION> key, uint32_t t)
    {
        ++n_;
        key_list.push_back(key);
    }
    size_t n_ = 0;
};

struct Counter_nyc_lookup
{
    void operator()(PhPoint<NYC_DIMENSION> key, uint32_t t)
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

void nyc()
{

    std::ifstream infile(NYC_DATA_ADDR);

    PhTree<NYC_DIMENSION, bool> tree;
    int n_points = 0;
    bool dummy = true;
    TimeStamp diff, start;
    diff = 0;

    std::string line;
    std::vector<PhPoint<NYC_DIMENSION>> point_vect;
    std::vector<TimeStamp> insertion_latency_vect;

    /**
     * Insertion
     */

    TimeStamp start_end_to_end = GetTimestamp();

    while (std::getline(infile, line))
    {
        PhPoint<NYC_DIMENSION> p1;
        std::vector<int32_t> vect = parse_line_nyc(line);
        p1[0] = n_points;

        for (int i = 0; i < NYC_DIMENSION - 1; i++)
        {
            p1[i + 1] = (uint32_t)vect[i];
        }

        if (n_points == 0)
        {
            for (int i = 0; i < NYC_DIMENSION; i++)
            {
                std::cout << p1[i] << " ";
            }
            std::cout << std::endl;
        }

        start = GetTimestamp();
        tree.emplace(p1, dummy);
        TimeStamp latency = GetTimestamp() - start;
        diff += latency;
        n_points++;

        if (n_points > NYC_SIZE - points_to_insert)
            insertion_latency_vect.push_back(latency + SERVER_TO_SERVER_IN_NS);

        if (n_points == NYC_SIZE)
            break;

        if (n_points % (NYC_SIZE / 100) == 0)
            std::cout << "n_points: " << n_points << std::endl;
    }

    std::cout << "Done! "
              << "Insertion Latency per point: " << (float)(GetTimestamp() - start_end_to_end) / n_points << std::endl;
    flush_vector_to_file(insertion_latency_vect, results_folder_addr + "/ph-tree/nyc_insert");

    /**
     * Point Lookup
     */

    start_end_to_end = GetTimestamp();
    std::vector<int32_t> max_values = {20160630, 20221220, 899, 898, 899, 898, 255, 198623000, 21474808, 1000, 1312, 3950589, 21474836, 138, 21474830};
    std::vector<int32_t> min_values = {20090101, 19700101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    max_values[0] -= 20090000;
    max_values[1] -= 19700000;
    min_values[0] -= 20090000;
    min_values[1] -= 19700000;

    PhPoint<NYC_DIMENSION> lowest;
    PhPoint<NYC_DIMENSION> heightest;
    std::vector<TimeStamp> lookup_latency_vect;
    TimeStamp cumulative = 0;

    for (int i = 0; i < NYC_DIMENSION - 1; i++)
    { // Make room for primary key
        lowest[i + 1] = (uint32_t)min_values[i];
        heightest[i + 1] = (uint32_t)max_values[i];
    }

    for (int i = 0; i < points_to_lookup; i++)
    {

        PhPoint<NYC_DIMENSION> start_range = lowest;
        PhPoint<NYC_DIMENSION> end_range = heightest;
        start_range[0] = i;
        end_range[0] = i;

        Counter_nyc_lookup callback;

        cnt = 0;
        cnt_traverse_node = 0;
        for_cnt = 0;

        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        TimeStamp temp_diff = GetTimestamp() - start;
        cumulative += temp_diff;

        if (found_cnt != 1 && lookup_pt[0] != i)
        {
            std::cout << "wrong number of points found! found_cnt: " << found_cnt << std::endl;
            for (const auto i : lookup_pt)
                std::cout << i << " " << std::endl;
            exit(-1);
        }

        if (i == 0)
        {
            for (int j = 0; j < NYC_DIMENSION; j++)
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
    flush_vector_to_file(lookup_latency_vect, results_folder_addr + "/ph-tree/nyc_lookup");

    /**
     * Range Search
     */

    std::ifstream file(NYC_QUERY_ADDR);
    std::ofstream outfile(results_folder_addr + "/ph-tree/nyc_query");

    max_values = {20160630, 20221220, 899, 898, 899, 898, 255, 198623000, 21474808, 1000, 1312, 3950589, 21474836, 138, 21474830};
    min_values = {20090101, 19700101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for (int i = 0; i < 1000; i++)
    {

        PhPoint<NYC_DIMENSION> start_range;
        PhPoint<NYC_DIMENSION> end_range;
        start_range[0] = 0;
        end_range[0] = NYC_SIZE;

        for (int i = 0; i < NYC_DIMENSION - 1; i++)
        {
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
            if (start_range_str != "-1")
            {
                if (index >= 2 && index <= 5)
                {
                    float num_float = std::stof(start_range_str);
                    start_range[static_cast<int32_t>(index) + 1] = static_cast<int32_t>(num_float * 10);
                }
                else
                    start_range[static_cast<int32_t>(index) + 1] = static_cast<int32_t>(std::stoul(start_range_str));
            }
            if (end_range_str != "-1")
            {
                if (index >= 2 && index <= 5)
                {
                    float num_float = std::stof(end_range_str);
                    end_range[static_cast<int32_t>(index) + 1] = static_cast<int32_t>(num_float * 10);
                }
                else
                    end_range[static_cast<int32_t>(index) + 1] = static_cast<int32_t>(std::stoul(end_range_str));
            }
        }

        start_range[0 + 1] -= 20090000;
        start_range[1 + 1] -= 19700000;
        end_range[0 + 1] -= 20090000;
        end_range[1 + 1] -= 19700000;

        Counter callback;
        start = GetTimestamp();
        tree.for_each({start_range, end_range}, callback);
        TimeStamp temp_diff = GetTimestamp() - start;

        outfile << "Query " << i << " end to end latency (ms): " << temp_diff / 1000 << ", found points count: " << key_list.size() << std::endl;
        key_list.clear();
    }
}

int main(int argc, char *argv[])
{

    std::cout << "nyc taxi......" << std::endl;
    nyc();
}