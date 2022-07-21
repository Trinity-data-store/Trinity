#ifndef TrinityNewBench_H
#define TrinityNewBench_H

#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "trie.h"
#include "TrinityParseFIle.h"
#include <future>
#include <atomic>
#include <tuple>
#include <iostream>
#include <fstream>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
int WARMUP_FACTOR = 5;
uint32_t points_to_skip;

std::vector<std::vector<int32_t>> load_dataset_vector_tpch(std::string file_address, uint32_t total_points) 
{
    std::ifstream file(file_address);
    std::vector<std::vector<int32_t>> points_vect;
    std::string line;
    for (uint32_t i = 0; i < total_points; i++){
        std::getline(file, line);
        points_vect.push_back(parse_line_tpch(line));
    }
    return points_vect;
}

std::vector<std::vector<int32_t>> load_dataset_vector_github(std::string file_address, uint32_t total_points) 
{
    std::ifstream file(file_address);
    std::vector<std::vector<int32_t>> points_vect;
    std::string line;
    for (uint32_t i = 0; i < total_points; i++){
        std::getline(file, line);
        points_vect.push_back(parse_line_github(line));
        // for (auto j : points_vect[i]) 
        //     std::cout << j << " ";
        // exit(0);
    }
    return points_vect;
}

uint32_t insert_each_client(std::vector<std::vector<int32_t>> *total_points_stored, int shard_number, int client_number, int client_index, std::vector<std::string> server_ips, uint32_t points_to_insert)
{
    auto client = MDTrieClient(server_ips, shard_number);
    uint32_t warmup_points = points_to_insert / WARMUP_FACTOR;

    TimeStamp start, diff = 0; 
    bool finished_warmup = false;
    uint32_t inserted_points_count = 0;
    for (uint32_t point_idx = client_index; point_idx < points_to_insert; point_idx += client_number){

        if (!finished_warmup && point_idx > warmup_points){
            start = GetTimestamp();
            finished_warmup = true;
        }
        vector <int32_t> data_point = (*total_points_stored)[point_idx];
        client.insert(data_point, point_idx);
        inserted_points_count ++;
    }
    diff = GetTimestamp() - start;
    return ((float) inserted_points_count / diff) * 1000000;
}

uint32_t total_client_insert(int shard_number, int client_number, std::vector<std::string> server_ips, int which_part = 0){

    std::string file_address;
    if (current_dataset_idx == 1)
        file_address = "/mntData/tpch_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 2)
        file_address = "/mntData/github_split_10/x" + std::to_string(which_part);

    uint32_t points_to_insert;
    points_to_insert = 100000000 / 100;
    if (current_dataset_idx == 2)
        points_to_insert = 82805630;

    if (current_dataset_idx == 2 && which_part == 9)
        points_to_insert = 82805625;

    std::vector<std::vector<int32_t>> total_points_stored;
    if (current_dataset_idx == 1)
        total_points_stored = load_dataset_vector_tpch(file_address, points_to_insert);
    if (current_dataset_idx == 2)
        total_points_stored = load_dataset_vector_github(file_address, points_to_insert);

    std::vector<std::future<uint32_t>> threads; 
    threads.reserve(client_number);
    for (int i = 0; i < client_number; i++){

        threads.push_back(std::async(insert_each_client, &total_points_stored, shard_number, client_number, i, server_ips, points_to_insert));
    }
    uint32_t total_throughput = 0;
    for (int i = 0; i < client_number; i++){
        total_throughput += threads[i].get();
    } 
    return total_throughput;  
}

uint32_t insert_lookup_each_client(std::vector<std::vector<int32_t>> *total_points_stored, int shard_number, int client_number, int client_index, std::vector<std::string> server_ips, uint32_t points_to_insert)
{
    auto client = MDTrieClient(server_ips, shard_number);
    // uint32_t warmup_points = points_to_skip + (points_to_insert - points_to_skip) / WARMUP_FACTOR;

    TimeStamp start, diff = 0; 
    bool finished_warmup = false;

    uint32_t op_count = 0;
    uint32_t effective_op_count = 0;

    for (uint32_t point_idx = client_index; point_idx < points_to_insert; point_idx += client_number){

        op_count ++;

        bool do_insert;
        if (op_count % 20 != 19 && point_idx > points_to_skip)
            do_insert = false;
        else
            do_insert = true;

        // if (!finished_warmup && point_idx > warmup_points){
        //     finished_warmup = true;
        // }
        // if (finished_warmup)
        if (!finished_warmup && point_idx > points_to_skip) {
            start = GetTimestamp();
            finished_warmup = true;
        }
        if (point_idx > points_to_skip) 
            effective_op_count ++;
        
        if (do_insert)
        {
            vector <int32_t> data_point = (*total_points_stored)[point_idx];
            client.insert_send(data_point, point_idx);
            client.insert_rec(point_idx);
        } else {
            client.primary_key_lookup_send(point_idx - points_to_skip);
            std::vector<int32_t> rec_vect;
            client.primary_key_lookup_rec(rec_vect, point_idx - points_to_skip);
        }
    }
    diff = GetTimestamp() - start;
    return ((float) effective_op_count / diff) * 1000000;
}

uint32_t total_client_insert_lookup(int shard_number, int client_number, std::vector<std::string> server_ips, int which_part = 0){

    std::string file_address;
    if (current_dataset_idx == 1)
        file_address = "/mntData/tpch_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 2)
        file_address = "/mntData/github_split_10/x" + std::to_string(which_part);

    uint32_t points_to_operate = 100000000 / 10; 
    points_to_skip = points_to_operate / 2;
    std::vector<std::vector<int32_t>> total_points_stored = load_dataset_vector_tpch(file_address, points_to_operate);

    std::vector<std::future<uint32_t>> threads; 
    threads.reserve(client_number);
    for (int i = 0; i < client_number; i++){
        threads.push_back(std::async(insert_lookup_each_client, &total_points_stored, shard_number, client_number, i, server_ips, points_to_operate));
    }
    uint32_t total_throughput = 0;
    for (int i = 0; i < client_number; i++){
        total_throughput += threads[i].get();
    } 
    return total_throughput;  
}


uint32_t insert_query_each_client(std::vector<std::vector<int32_t>> *total_points_stored, int shard_number, int client_number, int client_index, std::vector<std::string> server_ips, uint32_t points_to_insert, bool search_only)
{
    auto client = MDTrieClient(server_ips, shard_number);
    TimeStamp start, diff = 0; 
    bool finished_warmup = false;

    uint32_t op_count = 0;
    uint32_t effective_op_count = 0;
    std::vector<int32_t> max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 58063825, 19980802};
    std::vector<int32_t> min_values = {1, 90000, 0, 0, 19920102, 19920131, 19920103, 81300, 19920101};
    std::string line_range_search;
    std::string range_query_address = "/proj/trinity-PG0/Trinity/queries/tpch/tpch_query_new_converted";
    std::vector<std::string> line_list;
    std::ifstream file_range_search(range_query_address);

    while(std::getline(file_range_search, line_range_search)) {
        line_list.push_back(line_range_search);
    }
    // std::cout << "line_list.size(): " << line_list.size() << std::endl;

    for (uint32_t point_idx = client_index; point_idx < points_to_insert; point_idx += client_number){

        op_count ++;

        bool do_insert;
        if ((search_only || op_count % 20 != 19) && point_idx > points_to_skip)
            do_insert = false;
        else
            do_insert = true;

        if (!finished_warmup && point_idx > points_to_skip) {
            start = GetTimestamp();
            finished_warmup = true;
        }
        if (point_idx > points_to_skip) 
            effective_op_count ++;
        
        if (do_insert)
        {
            vector <int32_t> data_point = (*total_points_stored)[point_idx];
            client.insert_send(data_point, point_idx);
            client.insert_rec(point_idx);
        } else {

            std::vector<int32_t> found_points;
            std::vector<int32_t> start_range = min_values;
            std::vector<int32_t> end_range = max_values;
            line_range_search = line_list[point_idx % 1000];

            std::stringstream ss(line_range_search);

            while (ss.good()) {

                std::string index_str;
                std::getline(ss, index_str, ',');

                std::string start_range_str;
                std::getline(ss, start_range_str, ',');
                std::string end_range_str;
                std::getline(ss, end_range_str, ',');

                // cout << start_range_str << " " << end_range_str << endl;
                if (start_range_str != "-1") {
                start_range[static_cast<int32_t>(std::stoul(index_str))] = static_cast<int32_t>(std::stoul(start_range_str));
                }
                if (end_range_str != "-1") {
                end_range[static_cast<int32_t>(std::stoul(index_str))] = static_cast<int32_t>(std::stoul(end_range_str));
                }
            }
            for (dimension_t i = 0; i < start_range.size(); i++){
                if (i >= 4 && i != 7) {
                    start_range[i] -= 19000000;
                    end_range[i] -= 19000000;
                }
            }
            // client.range_search_trie(found_points, start_range, end_range);
            client.range_search_trie_send(start_range, end_range);
            client.range_search_trie_rec(found_points);

            effective_op_count += found_points.size() - 1;
            // std::cout << "op_count: " << op_count << ", found_points.size(): " << found_points.size() << std::endl;
            found_points.clear();
        }
    }
    diff = GetTimestamp() - start;
    return ((float) effective_op_count / diff) * 1000000;
}

uint32_t total_client_insert_query(int shard_number, int client_number, std::vector<std::string> server_ips, int which_part = 0){

    std::string file_address = "/mntData/tpch_split_10/x" + std::to_string(which_part);
    points_to_skip = 5000000;
    uint32_t points_to_operate = points_to_skip + 500; 
    std::vector<std::vector<int32_t>> total_points_stored = load_dataset_vector_tpch(file_address, points_to_operate);

    std::vector<std::future<uint32_t>> threads; 
    threads.reserve(client_number);
    for (int i = 0; i < client_number; i++){
        threads.push_back(std::async(insert_query_each_client, &total_points_stored, shard_number, client_number, i, server_ips, points_to_operate, false));
    }
    uint32_t total_throughput = 0;
    for (int i = 0; i < client_number; i++){
        total_throughput += threads[i].get();
    } 
    return total_throughput;  
}


uint32_t total_client_query(int shard_number, int client_number, std::vector<std::string> server_ips, int which_part = 0){

    std::string file_address = "/mntData/tpch_split_10/x" + std::to_string(which_part);
    points_to_skip = 5000000;
    uint32_t points_to_operate = points_to_skip + 500; 
    std::vector<std::vector<int32_t>> total_points_stored = load_dataset_vector_tpch(file_address, points_to_operate);

    std::vector<std::future<uint32_t>> threads; 
    threads.reserve(client_number);
    for (int i = 0; i < client_number; i++){
        threads.push_back(std::async(insert_query_each_client, &total_points_stored, shard_number, client_number, i, server_ips, points_to_operate, true));
    }
    uint32_t total_throughput = 0;
    for (int i = 0; i < client_number; i++){
        total_throughput += threads[i].get();
    } 
    return total_throughput;  
}


#endif //TrinityNewBench_H
