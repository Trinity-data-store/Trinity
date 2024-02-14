#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <sys/time.h>
#include <unordered_map>
#include <thread> // Include the thread header

#include "ctpl_stl.h"
#include "BBTree.h"

#define TPCH_DIMENSION 9
#define GITHUB_DIMENSION 10
#define NYC_DIMENSION 15

// #define TPCH_SIZE 250000000   // 250M
// #define GITHUB_SIZE 200000000 // 200M
// #define NYC_SIZE 200000000    // 200M

#define TPCH_SIZE 2500000   // 2.5M
#define GITHUB_SIZE 2000000 // 2M
#define NYC_SIZE 2000000    // 2M

#define SKIP_SIZE 700000000 - GITHUB_SIZE
#define SERVER_TO_SERVER_IN_NS 92

int points_to_insert = 30000;
int points_to_lookup = 300; // Reduced, because slow

std::string ROOT_DIR = "/proj/trinity-PG0/Trinity/";
std::string TPCH_DATA_ADDR = ROOT_DIR + "datasets/tpch_dataset.csv";
std::string GITHUB_DATA_ADDR = ROOT_DIR + "datasets/github_dataset.csv";
std::string NYC_DATA_ADDR = ROOT_DIR + "datasets/nyc_dataset.csv";
std::string TPCH_QUERY_ADDR = ROOT_DIR + "queries/tpch_query";
std::string GITHUB_QUERY_ADDR = ROOT_DIR + "queries/github_query";
std::string NYC_QUERY_ADDR = ROOT_DIR + "queries/nyc_query";
std::string results_folder_addr = "/proj/trinity-PG0/Trinity/results/";

typedef unsigned long long int TimeStamp;

TimeStamp GetTimestamp()
{
    struct timeval now;
    gettimeofday(&now, nullptr);

    return now.tv_usec + (TimeStamp)now.tv_sec * 1000000;
}

void flush_vector_to_file(std::vector<TimeStamp> vect, std::string filename)
{
    std::ofstream outFile(filename);
    for (const auto &e : vect)
        outFile << std::to_string(e) << "\n";
}

std::vector<float>
parse_line_tpch(std::string line)
{
    std::vector<float> point(TPCH_DIMENSION, 0);
    int index = -1;
    bool primary_key = true;
    std::string delim = ",";
    auto start = 0U;
    auto end = line.find(delim);

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
        point[index] = static_cast<float>(std::stoul(substr));
    }
    index++;
    std::string substr = line.substr(start, end - start);
    point[index] = static_cast<float>(std::stoul(substr));

    for (int i = 0; i < TPCH_DIMENSION; i++)
    {
        if (i >= 4 && i != 7)
        {
            point[i] -= 19000000;
        }
    }
    return point;
}

std::vector<float>
parse_line_github(std::string line)
{
    std::vector<float> point(GITHUB_DIMENSION, 0);
    int index = -1;
    bool primary_key = true;
    std::string delim = ",";
    auto start = 0U;
    auto end = line.find(delim);

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
        point[index] = static_cast<float>(std::stoul(substr));
    }
    index++;
    std::string substr = line.substr(start, end - start);
    point[index] = static_cast<float>(std::stoul(substr));

    for (int i = 0; i < GITHUB_DIMENSION; i++)
    {
        if (i == 8 || i == 9)
        {
            point[i] -= 20110000;
        }
    }
    return point;
}

std::vector<float>
parse_line_nyc(std::string line)
{
    std::vector<float> point(NYC_DIMENSION, 0);
    int index = -1;
    bool primary_key = true;
    std::string delim = ",";
    auto start = 0U;
    auto end = line.find(delim);

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
        if (index >= 2 && index <= 5)
        {
            point[index] = std::stof(substr) * 10;
        }
        else
        {
            point[index] = std::stof(substr);
        }
    }
    index++;
    std::string substr = line.substr(start, end - start);
    point[index] = static_cast<float>(std::stoul(substr));
    point[0] -= 20090000;
    point[1] -= 19700000;
    return point;
}

void tpch()
{
    std::cout << "tpch started! " << std::endl;
    std::ifstream infile(TPCH_DATA_ADDR);
    std::string line;
    int n_points = 0;

    BBTree bbtree(TPCH_DIMENSION + 1);
    std::vector<TimeStamp> insertion_latency_vect;

    std::vector<std::vector<float>> bulk_insert_points;
    std::vector<uint32_t> bulk_insert_object_ids;
    while (std::getline(infile, line))
    {
        std::vector<float> vect = parse_line_tpch(line);
        vect.insert(vect.begin(), n_points);
        bulk_insert_points.push_back(vect);
        bulk_insert_object_ids.push_back(n_points);
        n_points++;

        if (n_points % (TPCH_SIZE / 100) == 0)
            std::cout << "n_points: " << n_points << std::endl;

        if (n_points == TPCH_SIZE - points_to_insert)
            break;
    }
    bbtree.BulkInsert(bulk_insert_points, bulk_insert_object_ids);
    std::cout << "Finish BulkInsert" << std::endl;

    TimeStamp start_end_to_end = GetTimestamp();
    while (std::getline(infile, line))
    {
        std::vector<float> vect = parse_line_tpch(line);
        vect.insert(vect.begin(), n_points);
        TimeStamp start = GetTimestamp();
        bbtree.InsertObject(vect, (uint32_t)n_points);
        TimeStamp latency = GetTimestamp() - start;
        n_points++;

        if (n_points > TPCH_SIZE - points_to_insert)
            insertion_latency_vect.push_back(latency + SERVER_TO_SERVER_IN_NS);
        if (n_points > TPCH_SIZE)
            break;
    }

    std::cout << "Done! "
              << "Insertion Latency per point: " << (float)(GetTimestamp() - start_end_to_end) / points_to_insert << std::endl;
    flush_vector_to_file(insertion_latency_vect, results_folder_addr + "/bb-tree/tpch_insert");

    std::vector<int32_t> max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 58063825, 19980802};
    std::vector<int32_t> min_values = {1, 90000, 0, 0, 19920102, 19920131, 19920103, 81300, 19920101};

    std::ifstream file(TPCH_QUERY_ADDR);
    std::ofstream outfile(results_folder_addr + "/bb-tree/tpch_query");

    for (int i = 0; i < 1000; i++)
    {
        std::vector<float> start_range;
        std::vector<float> end_range;

        for (int i = 0; i < TPCH_DIMENSION; i++)
        {
            start_range.push_back((float)min_values[i]);
            end_range.push_back((float)max_values[i]);
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

            if (start_range_str != "-1")
            {
                start_range[std::stoi(index_str)] = static_cast<float>(std::stoi(start_range_str));
            }
            if (end_range_str != "-1")
            {
                end_range[std::stoi(index_str)] = static_cast<float>(std::stoi(end_range_str));
            }
        }
        for (int i = 0; i < TPCH_DIMENSION; i++)
        {
            if (i >= 4 && i != 7)
            {
                start_range[i] -= 19000000;
                end_range[i] -= 19000000;
            }
        }
        start_range.insert(start_range.begin(), 0);
        end_range.insert(end_range.begin(), n_points);
        TimeStamp start = GetTimestamp();
        std::vector<uint32_t> results = bbtree.SearchRange(start_range,
                                                           end_range);
        TimeStamp temp_diff = GetTimestamp() - start;
        outfile << "Query " << i << " end to end latency (ms): " << temp_diff / 1000 << ", found points count: " << results.size() << std::endl;
    }

    std::vector<TimeStamp> lookup_latency_vect;
    start_end_to_end = GetTimestamp();
    for (int i = 0; i < points_to_lookup; i++)
    {
        std::vector<float> start_range;
        std::vector<float> end_range;

        for (int i = 0; i < TPCH_DIMENSION; i++)
        {
            start_range.push_back((float)min_values[i]);
            end_range.push_back((float)max_values[i]);
        }

        for (int i = 0; i < TPCH_DIMENSION; i++)
        {
            if (i >= 4 && i != 7)
            {
                start_range[i] -= 19000000;
                end_range[i] -= 19000000;
            }
        }
        start_range.insert(start_range.begin(), i);
        end_range.insert(end_range.begin(), i);
        TimeStamp start = GetTimestamp();
        std::vector<uint32_t> results = bbtree.SearchRange(start_range,
                                                           end_range);
        TimeStamp temp_diff = GetTimestamp() - start;
        lookup_latency_vect.push_back(temp_diff + SERVER_TO_SERVER_IN_NS);
    }
    std::cout << "Done! "
              << "Lookup Latency per point: " << (float)(GetTimestamp() - start_end_to_end) / points_to_lookup << std::endl;
    flush_vector_to_file(lookup_latency_vect, results_folder_addr + "/bb-tree/tpch_lookup");
}

void github()
{
    std::ifstream infile(GITHUB_DATA_ADDR);
    std::string line;
    TimeStamp diff, start;
    int n_points = 0;

    BBTree bbtree(GITHUB_DIMENSION + 1);
    int skip_lines = 0;
    std::vector<TimeStamp> insertion_latency_vect;

    std::vector<std::vector<float>> bulk_insert_points;
    std::vector<uint32_t> bulk_insert_object_ids;
    while (std::getline(infile, line))
    {
        if (skip_lines < SKIP_SIZE)
        {
            skip_lines += 1;
            continue;
        }
        std::vector<float> vect = parse_line_github(line);
        vect.insert(vect.begin(), n_points);
        bulk_insert_points.push_back(vect);
        bulk_insert_object_ids.push_back(n_points);
        n_points++;

        if (n_points % (GITHUB_SIZE / 100) == 0)
            std::cout << "n_points: " << n_points << std::endl;

        if (n_points == GITHUB_SIZE - points_to_insert)
            break;
    }
    bbtree.BulkInsert(bulk_insert_points, bulk_insert_object_ids);
    std::cout << "Finish BulkInsert" << std::endl;

    TimeStamp start_end_to_end = GetTimestamp();
    while (std::getline(infile, line))
    {
        if (skip_lines < SKIP_SIZE)
        {
            skip_lines += 1;
            continue;
        }
        std::vector<float> vect = parse_line_github(line);
        vect.insert(vect.begin(), n_points);
        start = GetTimestamp();
        bbtree.InsertObject(vect, (uint32_t)n_points);
        TimeStamp latency = GetTimestamp() - start;
        diff += latency;
        n_points++;

        if (n_points > GITHUB_SIZE - points_to_insert)
            insertion_latency_vect.push_back(latency + SERVER_TO_SERVER_IN_NS);
        if (n_points > GITHUB_SIZE)
            break;
    }

    std::cout << "Done! "
              << "Insertion Latency per point: " << (float)(GetTimestamp() - start_end_to_end) / points_to_insert << std::endl;
    flush_vector_to_file(insertion_latency_vect, results_folder_addr + "/bb-tree/github_insert");

    std::vector<int32_t> max_values = {7451541, 737170, 262926, 354850, 379379, 3097263, 703341, 8745, 20201206 - 20110000, 20201206 - 20110000};
    std::vector<int32_t> min_values = {1, 1, 0, 0, 0, 0, 0, 0, 20110211 - 20110000, 20110211 - 20110000};

    std::ifstream file(GITHUB_QUERY_ADDR);
    std::ofstream outfile(results_folder_addr + "/bb-tree/github_query");
    for (int i = 0; i < 1000; i++)
    {
        std::vector<float> start_range;
        std::vector<float> end_range;

        for (int i = 0; i < GITHUB_DIMENSION; i++)
        {
            start_range.push_back((float)min_values[i]);
            end_range.push_back((float)max_values[i]);
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

            if (start_range_str != "-1")
            {
                start_range[std::stoi(index_str)] = static_cast<float>(std::stoi(start_range_str));
            }
            if (end_range_str != "-1")
            {
                end_range[std::stoi(index_str)] = static_cast<float>(std::stoi(end_range_str));
            }
        }

        for (int i = 1; i < GITHUB_DIMENSION; i++)
        {
            if (i >= 9)
            {
                start_range[i] -= 20110000;
                end_range[i] -= 20110000;
            }
        }
        start_range.insert(start_range.begin(), 0);
        end_range.insert(end_range.begin(), n_points);
        start = GetTimestamp();
        std::vector<uint32_t> results = bbtree.SearchRange(start_range,
                                                           end_range);
        TimeStamp temp_diff = GetTimestamp() - start;
        outfile << "Query " << i << " end to end latency (ms): " << temp_diff / 1000 << ", found points count: " << results.size() << std::endl;
    }

    std::vector<TimeStamp> lookup_latency_vect;
    start_end_to_end = GetTimestamp();
    for (int i = 0; i < points_to_lookup; i++)
    {
        std::vector<float> start_range;
        std::vector<float> end_range;

        for (int i = 0; i < GITHUB_DIMENSION; i++)
        {
            start_range.push_back((float)min_values[i]);
            end_range.push_back((float)max_values[i]);
        }
        for (int i = 1; i < GITHUB_DIMENSION; i++)
        {
            if (i >= 9)
            {
                start_range[i] -= 20110000;
                end_range[i] -= 20110000;
            }
        }
        start_range.insert(start_range.begin(), i);
        end_range.insert(end_range.begin(), i);
        start = GetTimestamp();
        std::vector<uint32_t> results = bbtree.SearchRange(start_range,
                                                           end_range);
        TimeStamp temp_diff = GetTimestamp() - start;
        lookup_latency_vect.push_back(temp_diff + SERVER_TO_SERVER_IN_NS);
    }
    std::cout << "Done! "
              << "Lookup Latency per point: " << (float)(GetTimestamp() - start_end_to_end) / points_to_lookup << std::endl;
    flush_vector_to_file(lookup_latency_vect, results_folder_addr + "/bb-tree/github_lookup");
}

void nyc()
{
    std::ifstream infile(NYC_DATA_ADDR);
    std::string line;
    TimeStamp diff, start;
    int n_points = 0;

    BBTree bbtree(NYC_DIMENSION + 1);
    std::vector<TimeStamp> insertion_latency_vect;

    std::vector<std::vector<float>> bulk_insert_points;
    std::vector<uint32_t> bulk_insert_object_ids;
    while (std::getline(infile, line))
    {
        std::vector<float> vect = parse_line_nyc(line);
        vect.insert(vect.begin(), n_points);
        bulk_insert_points.push_back(vect);
        bulk_insert_object_ids.push_back(n_points);
        n_points++;

        if (n_points % (NYC_SIZE / 100) == 0)
            std::cout << "n_points: " << n_points << std::endl;

        if (n_points == NYC_SIZE - points_to_insert)
            break;
    }
    bbtree.BulkInsert(bulk_insert_points, bulk_insert_object_ids);
    std::cout << "Finish BulkInsert" << std::endl;

    TimeStamp start_end_to_end = GetTimestamp();
    while (std::getline(infile, line))
    {
        std::vector<float> vect = parse_line_nyc(line);
        vect.insert(vect.begin(), n_points);
        start = GetTimestamp();
        bbtree.InsertObject(vect, (uint32_t)n_points);
        TimeStamp latency = GetTimestamp() - start;
        diff += latency;
        n_points++;

        if (n_points > NYC_SIZE - points_to_insert)
            insertion_latency_vect.push_back(latency + SERVER_TO_SERVER_IN_NS);
        if (n_points > NYC_SIZE)
            break;
    }
    std::cout << "Done! "
              << "Insertion Latency per point: " << (float)(GetTimestamp() - start_end_to_end) / points_to_insert << std::endl;
    flush_vector_to_file(insertion_latency_vect, results_folder_addr + "/bb-tree/nyc_insert");

    std::vector<int32_t> max_values = {20160630, 20221220, 899, 898, 899, 898, 255, 198623000, 21474808, 1000, 1312, 3950589, 21474836, 138, 21474830};
    std::vector<int32_t> min_values = {20090101, 19700101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::ifstream file(NYC_QUERY_ADDR);
    std::ofstream outfile(results_folder_addr + "/bb-tree/nyc_query");

    max_values[0] -= 20090000;
    max_values[1] -= 19700000;
    min_values[0] -= 20090000;
    min_values[1] -= 19700000;

    for (int i = 0; i < 1000; i++)
    {

        std::vector<float> start_range;
        std::vector<float> end_range;

        for (int i = 0; i < NYC_DIMENSION; i++)
        {
            start_range.push_back((float)min_values[i]);
            end_range.push_back((float)max_values[i]);
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

            if (start_range_str != "-1")
            {
                start_range[std::stoi(index_str)] = static_cast<float>(std::stoi(start_range_str));
            }
            if (end_range_str != "-1")
            {
                end_range[std::stoi(index_str)] = static_cast<float>(std::stoi(end_range_str));
            }
        }

        start_range.insert(start_range.begin(), 0);
        end_range.insert(end_range.begin(), n_points);
        start = GetTimestamp();
        std::vector<uint32_t> results = bbtree.SearchRange(start_range,
                                                           end_range);
        TimeStamp temp_diff = GetTimestamp() - start;
        outfile << "Query " << i << " end to end latency (ms): " << temp_diff / 1000 << ", found points count: " << results.size() << std::endl;
    }

    std::vector<TimeStamp> lookup_latency_vect;
    start_end_to_end = GetTimestamp();
    for (int i = 0; i < points_to_lookup; i++)
    {

        std::vector<float> start_range;
        std::vector<float> end_range;

        for (int i = 0; i < NYC_DIMENSION; i++)
        {
            start_range.push_back((float)min_values[i]);
            end_range.push_back((float)max_values[i]);
        }
        start_range.insert(start_range.begin(), i);
        end_range.insert(end_range.begin(), i);
        start = GetTimestamp();
        std::vector<uint32_t> results = bbtree.SearchRange(start_range,
                                                           end_range);
        TimeStamp temp_diff = GetTimestamp() - start;
        lookup_latency_vect.push_back(temp_diff + SERVER_TO_SERVER_IN_NS);
    }
    std::cout << "Done! "
              << "Lookup Latency per point: " << (float)(GetTimestamp() - start_end_to_end) / points_to_lookup << std::endl;
    flush_vector_to_file(lookup_latency_vect, results_folder_addr + "/bb-tree/nyc_lookup");
}

int main(int argc, char *argv[])
{
    srand(0);

    tpch();
    std::cout << "tpch done! " << std::endl;

    github();
    std::cout << "github done!" << std::endl;

    nyc();
    std::cout << "nyc done!" << std::endl;
}