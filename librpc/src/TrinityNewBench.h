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

    std::string file_address = "/mntData/tpch_split_10/x" + std::to_string(which_part);
    uint32_t points_to_insert = 100000000;
    std::vector<std::vector<int32_t>> total_points_stored = load_dataset_vector_tpch(file_address, points_to_insert);

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

#endif //TrinityNewBench_H