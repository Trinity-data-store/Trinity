#ifndef TrinityNewBench_H
#define TrinityNewBench_H

#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "trie.h"
#include "TrinityParseFile.h"
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
std::vector<int> pkey_vect;

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
    }
    return points_vect;
}

std::vector<std::vector<int32_t>> load_dataset_vector_github_query(std::string file_address, uint32_t total_points) 
{
    std::ifstream file(file_address);
    std::vector<std::vector<int32_t>> points_vect;
    std::string line;
    uint32_t skip = 75000000;
    for (uint32_t i = 0; i < total_points + skip; i++){
        if (i >= skip) {
            std::getline(file, line);
            points_vect.push_back(parse_line_github(line));
        }
    }
    return points_vect;
}


std::vector<std::vector<int32_t>> load_dataset_vector_nyc(std::string file_address, uint32_t total_points) 
{
    std::ifstream file(file_address);
    std::vector<std::vector<int32_t>> points_vect;
    std::string line;
    for (uint32_t i = 0; i < total_points; i++){
        std::getline(file, line);
        points_vect.push_back(parse_line_nyc(line));
    }
    return points_vect;
}

std::vector<int32_t> load_pkey_vector_nyc(std::string file_address, uint32_t total_points) 
{
    std::ifstream file(file_address);
    std::vector<int32_t> pkey_vect;
    std::string line;
    for (uint32_t i = 0; i < total_points; i++){
        std::getline(file, line);
        std::string delim = ",";
        auto start = 0U;
        auto end = line.find(delim);
        std::string substr = line.substr(start, end - start); 
        pkey_vect.push_back(std::stoul(substr));
    }
    return pkey_vect;
}

void flush_vector_to_file(std::vector<TimeStamp> vect, std::string filename){
    std::ofstream outFile(filename);
    for (const auto &e : vect) outFile << std::to_string(e) << "\n";
}

uint32_t insertion_latency_bench (std::vector<std::string> server_ips, int shard_number, int which_part = 0)
{
    std::string file_address;
    if (current_dataset_idx == 1)
        file_address = "/mntData/tpch_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 2)
        file_address = "/mntData/github_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 3)
        file_address = "/mntData/nyc_split_10/x" + std::to_string(which_part);

    uint32_t points_to_insert = 30000;
    std::vector<std::vector<int32_t>> total_points_stored;
    if (current_dataset_idx == 1)
        total_points_stored = load_dataset_vector_tpch(file_address, points_to_insert);
    if (current_dataset_idx == 2)
        total_points_stored = load_dataset_vector_github(file_address, points_to_insert);
    if (current_dataset_idx == 3) {
        total_points_stored = load_dataset_vector_nyc(file_address, points_to_insert);
    }

    /*
        Insertion
    */
    std::vector<TimeStamp> latency_list;
    auto client = MDTrieClient(server_ips, shard_number);
    uint32_t warmup_points = points_to_insert / WARMUP_FACTOR;

    TimeStamp cumulative = 0; 
    bool finished_warmup = false;
    uint32_t inserted_points_count = 0;
    for (uint32_t point_idx = 0; point_idx < points_to_insert; point_idx += 1){

        if (!finished_warmup && point_idx > warmup_points){
            finished_warmup = true;
            inserted_points_count = 0;
        }
        vector <int32_t> data_point = total_points_stored[point_idx];
        TimeStamp start = GetTimestamp();
        client.insert_for_latency(data_point, point_idx);
        if (finished_warmup) {
            TimeStamp latency = GetTimestamp() - start;
            cumulative += latency;
            latency_list.push_back(latency);
        }

        inserted_points_count ++;
    }

    if (current_dataset_idx == 1)
        flush_vector_to_file(latency_list, "/proj/trinity-PG0/Trinity/results/latency_cdf/trinity_tpch_insert");
    if (current_dataset_idx == 2)
        flush_vector_to_file(latency_list, "/proj/trinity-PG0/Trinity/results/latency_cdf/trinity_github_insert");
    if (current_dataset_idx == 3)
        flush_vector_to_file(latency_list, "/proj/trinity-PG0/Trinity/results/latency_cdf/trinity_nyc_insert");

    std::vector<int32_t> server_latency_list;
    client.get_insert_latency(server_latency_list);
    int32_t cumulative_server = 0;
    for (const auto l : server_latency_list)
        cumulative_server += l;
    std::cout << "server latency: " << cumulative_server / server_latency_list.size() << std::endl;

    return cumulative / inserted_points_count;
}

uint32_t lookup_latency_bench (std::vector<std::string> server_ips, int shard_number, int which_part = 0)
{
    std::string file_address;
    if (current_dataset_idx == 1)
        file_address = "/mntData/tpch_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 2)
        file_address = "/mntData/github_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 3)
        file_address = "/mntData/nyc_split_10/x" + std::to_string(which_part);

    // std::cout << "Loading dataset " << endl; 
    uint32_t points_to_lookup = 30000;
    std::vector<std::vector<int32_t>> total_points_stored;
    if (current_dataset_idx == 1)
        total_points_stored = load_dataset_vector_tpch(file_address, points_to_lookup);
    if (current_dataset_idx == 2)
        total_points_stored = load_dataset_vector_github(file_address, points_to_lookup);
    if (current_dataset_idx == 3) {
        total_points_stored = load_dataset_vector_nyc(file_address, points_to_lookup);
    }

    // std::cout << "Loaded dataset " << endl; 

    /*
        Lookup
    */
    std::vector<TimeStamp> latency_list;
    auto client = MDTrieClient(server_ips, shard_number);
    uint32_t warmup_points = points_to_lookup / WARMUP_FACTOR;

    TimeStamp cumulative = 0; 
    bool finished_warmup = false;
    uint32_t lookup_points_count = 0;
    for (uint32_t point_idx = 0; point_idx < points_to_lookup; point_idx += 1){

        if (!finished_warmup && point_idx > warmup_points){
            finished_warmup = true;
            lookup_points_count = 0;
        }
        std::vector<int32_t> rec_vect;
        // std::cout << "primary_key_lookup_send_zipf:  " << point_idx << endl; 

        TimeStamp start = GetTimestamp();
        client.primary_key_lookup_send_zipf(point_idx / 100 /*total server shards*/, point_idx);
        client.primary_key_lookup_rec_zipf(rec_vect, point_idx);
        // std::cout << "primary_key_lookup_rec_zipf:  " << point_idx << endl; 

        if (finished_warmup) {
            TimeStamp latency = GetTimestamp() - start;
            cumulative += latency;
            latency_list.push_back(latency);
        }


        std::vector<int32_t> correct_vect = total_points_stored[point_idx];
        // std::cout << "correct_vect.size():  " << correct_vect.size() << endl; 

        /*
        for (unsigned int i = 0; i < correct_vect.size(); i ++) {
            if (correct_vect[i] != rec_vect[i]) {
                std::cerr << "lookup failed!" << std::endl;
                std::cout << "correct vect:" << std::endl;
                for (auto i : correct_vect) {
                    std::cout << i << ",";
                }
                std::cout << std::endl << "queried vect:" << std::endl;
                for (auto i : rec_vect) {
                    std::cout << i << ",";
                }
                std::cout << std::endl;
                exit(0);
            }
        }
        */
        lookup_points_count ++;
        // if (lookup_points_count % 100 == 0)
        //     std::cout << "lookup_pts_count: " << lookup_points_count << std::endl;
    }

    if (current_dataset_idx == 1)
        flush_vector_to_file(latency_list, "/proj/trinity-PG0/Trinity/results/latency_cdf/trinity_tpch_lookup");
    if (current_dataset_idx == 2)
        flush_vector_to_file(latency_list, "/proj/trinity-PG0/Trinity/results/latency_cdf/trinity_github_lookup");
    if (current_dataset_idx == 3)
        flush_vector_to_file(latency_list, "/proj/trinity-PG0/Trinity/results/latency_cdf/trinity_nyc_lookup");

    std::vector<int32_t> server_latency_list;
    client.get_lookup_latency(server_latency_list);
    int32_t cumulative_server = 0;
    for (const auto l : server_latency_list)
        cumulative_server += l;
    std::cout << "server latency: " << cumulative_server / server_latency_list.size() << std::endl;

    return cumulative / lookup_points_count;
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
            inserted_points_count = 0;
        }
        vector <int32_t> data_point = (*total_points_stored)[point_idx];
        client.insert(data_point, point_idx);
        inserted_points_count ++;
    }
    diff = GetTimestamp() - start;
    return ((float) inserted_points_count / diff) * 1000000;
}

std::vector<int> primary_key_zipfan(uint32_t total_points) {

    std::string file_address = "/proj/trinity-PG0/Trinity/queries/zipf_keys_30m";
    std::ifstream file(file_address);
    std::vector<int> pkey_vect;
    std::string line;
    for (uint32_t i = 0; i < total_points; i++){
        std::getline(file, line);
        pkey_vect.push_back(std::stoi(line));
    }
    return pkey_vect;
}

uint32_t total_client_insert(int shard_number, int client_number, std::vector<std::string> server_ips, int which_part = 0){

    std::string file_address;
    if (current_dataset_idx == 1)
        file_address = "/mntData/tpch_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 2)
        file_address = "/mntData/github_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 3)
        file_address = "/mntData/nyc_split_10/x" + std::to_string(which_part);

    uint32_t points_to_insert;

    if (current_dataset_idx == 1) {
        points_to_insert = 100000000;
        // points_to_insert = 30000000;
    }
        
    if (current_dataset_idx == 2) {
        points_to_insert = 82805630;
        if (which_part == 9)
            points_to_insert = 82805625;
        points_to_insert = 30000000;
    }

    if (current_dataset_idx == 3) {
        // points_to_insert = 67520000;
        points_to_insert = 30000000; 
    }
    

    std::vector<std::vector<int32_t>> total_points_stored;
    if (current_dataset_idx == 1)
        total_points_stored = load_dataset_vector_tpch(file_address, points_to_insert);
    if (current_dataset_idx == 2)
        total_points_stored = load_dataset_vector_github(file_address, points_to_insert);
    if (current_dataset_idx == 3) {
        total_points_stored = load_dataset_vector_nyc(file_address, points_to_insert);
    }

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

uint32_t lookup_each_client(int shard_number, int client_number, int client_index, std::vector<std::string> server_ips, uint32_t points_to_lookup, std::vector<int> const &pkey_vect, int which_part)
{
    // std::cout << "lookup_each_client: " << shard_number << ", " << client_number << ", " << client_index << std::endl;
    auto client = MDTrieClient(server_ips, shard_number);
    uint32_t warmup_points = points_to_lookup / WARMUP_FACTOR;

    TimeStamp start, diff = 0; 
    bool finished_warmup = false;
    uint32_t lookup_points_count = 0;
    uint32_t lookup_idx = 0;
    for (uint32_t point_idx = client_index + which_part; point_idx < points_to_lookup; point_idx += client_number){
        
        // if (lookup_points_count % 10 == 0)
        //     std::cout << "lookup_points_count: " << lookup_points_count << std::endl;

        if (!finished_warmup && point_idx > warmup_points){
            start = GetTimestamp();
            finished_warmup = true;
            lookup_points_count = 0;

        }
        std::vector<int32_t> rec_vect;
        // std::string return_str;

        // client.primary_key_lookup_send(lookup_idx);
        // client.primary_key_lookup_rec(rec_vect, lookup_idx);

        uint32_t pkey = pkey_vect[point_idx];
        // pkey = point_idx;
        // std::cout << pkey << std::endl;
        client.primary_key_lookup_send_zipf(pkey, point_idx);
        client.primary_key_lookup_rec_zipf(rec_vect, point_idx);
        // return 0;
        // client.primary_key_lookup_binary_send(lookup_idx);
        // client.primary_key_lookup_binary_rec(return_str, lookup_idx);
        lookup_points_count ++;
        lookup_idx ++;
    }
    diff = GetTimestamp() - start;
    return ((float) lookup_points_count / diff) * 1000000;
}


uint32_t lookup_each_client_async(int shard_number, int client_number, int client_index, std::vector<std::string> server_ips, uint32_t points_to_lookup, std::vector<int> const &pkey_vect){

    auto client = MDTrieClient(server_ips, shard_number);

    uint32_t warmup_points = points_to_lookup / WARMUP_FACTOR;

    int sent_count = 0;
    TimeStamp start, diff = 0; 
    bool finished_warmup = false;
    uint32_t lookup_points_count = 0;
    uint32_t lookup_idx = 0;
    uint32_t point_idx;
    for (point_idx = client_index; point_idx < points_to_lookup; point_idx += client_number){

        if (!finished_warmup && point_idx > warmup_points){
            start = GetTimestamp();
            finished_warmup = true;
            lookup_points_count = 0;
        }

        if (sent_count != 0 && sent_count % 5 == 0){
            for (uint32_t j = point_idx - sent_count * client_number; j < point_idx; j += client_number){
                std::vector<int32_t> rec_vect;
                client.primary_key_lookup_rec(rec_vect, pkey_vect[j]);
            }
            sent_count = 0;
        }

        client.primary_key_lookup_send(pkey_vect[point_idx]);
        sent_count ++;
        lookup_points_count ++;
        lookup_idx ++;
    }

    for (uint32_t j = point_idx - sent_count * client_number; j < point_idx; j += client_number){
        std::vector<int32_t> rec_vect;
        client.primary_key_lookup_rec(rec_vect, pkey_vect[j]);
    }
    diff = GetTimestamp() - start;
    return ((float) lookup_points_count / diff) * 1000000;
}


uint32_t total_client_lookup(int shard_number, int client_number, std::vector<std::string> server_ips, int which_part = 0){

    std::string file_address;
    if (current_dataset_idx == 1)
        file_address = "/mntData/tpch_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 2)
        file_address = "/mntData/github_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 3)
        file_address = "/mntData/nyc_split_10/x" + std::to_string(which_part);

    uint32_t points_to_lookup;

    if (current_dataset_idx == 1) {
        points_to_lookup = 100000000;
        points_to_lookup = 30000000; // See Aerospike
    }
        
    if (current_dataset_idx == 2) {
        points_to_lookup = 82805630;
        if (which_part == 9)
            points_to_lookup = 82805625;
        points_to_lookup = 30000000; // See Aerospike
    }

    if (current_dataset_idx == 3) {
        points_to_lookup = 67520000;
        points_to_lookup = 30000000; 
    }
    pkey_vect = primary_key_zipfan(points_to_lookup);
    // std::vector<int> pkey_value_vect = load_pkey_vector_nyc(file_address, points_to_lookup);
    // for (int i = 0; i < 10; i ++) {
    //     std::cout << pkey_value_vect[pkey_vect[i]] << ",";
    // }
    
    // std::cout << "\n";
    // exit(0);

    std::vector<std::future<uint32_t>> threads; 
    threads.reserve(client_number);
    for (int i = 0; i < client_number; i++){

        threads.push_back(std::async(lookup_each_client, shard_number, client_number, i, server_ips, points_to_lookup, pkey_vect, which_part));
    }
    uint32_t total_throughput = 0;
    for (int i = 0; i < client_number; i++){
        total_throughput += threads[i].get();
    } 
    return total_throughput;  
}


uint32_t insert_lookup_each_client(std::vector<std::vector<int32_t>> *total_points_stored, int shard_number, int client_number, int client_index, std::vector<std::string> server_ips, uint32_t points_to_insert, bool is_50 = false)
{
    auto client = MDTrieClient(server_ips, shard_number);

    TimeStamp start, diff = 0; 
    bool finished_warmup = false;

    uint32_t op_count = 0;
    uint32_t effective_op_count = 0;
    int factor = 20;
    if (is_50) 
        factor = 2;
    for (uint32_t point_idx = client_index; point_idx < points_to_insert; point_idx += client_number){

        op_count ++;

        bool do_insert;
        if (op_count % factor != 1 && point_idx > points_to_skip)
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
            std::vector<int32_t> rec_vect;
            uint32_t pkey = pkey_vect[point_idx - points_to_skip];
            client.primary_key_lookup_send_zipf(pkey, point_idx - points_to_skip);
            client.primary_key_lookup_rec_zipf(rec_vect, point_idx - points_to_skip);

        }
    }
    diff = GetTimestamp() - start;
    return ((float) effective_op_count / diff) * 1000000;
}

uint32_t total_client_insert_lookup(int shard_number, int client_number, std::vector<std::string> server_ips, int which_part = 0, bool is_50 = false){

    std::string file_address;
    if (current_dataset_idx == 1)
        file_address = "/mntData/tpch_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 2)
        file_address = "/mntData/github_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 3)
        file_address = "/mntData/nyc_split_10/x" + std::to_string(which_part);

    points_to_skip = 10000000;
    uint32_t points_to_operate = points_to_skip + points_to_skip / 10; 
    pkey_vect = primary_key_zipfan(points_to_operate);

    std::vector<std::vector<int32_t>> total_points_stored;

    if (current_dataset_idx == 1)
        total_points_stored = load_dataset_vector_tpch(file_address, points_to_operate);
    if (current_dataset_idx == 2) {
        total_points_stored = load_dataset_vector_github(file_address, points_to_operate);
    }
    if (current_dataset_idx == 3) {
        total_points_stored = load_dataset_vector_nyc(file_address, points_to_operate);
    }

    std::vector<std::future<uint32_t>> threads; 
    threads.reserve(client_number);
    for (int i = 0; i < client_number; i++){
        threads.push_back(std::async(insert_lookup_each_client, &total_points_stored, shard_number, client_number, i, server_ips, points_to_operate, is_50));
    }
    uint32_t total_throughput = 0;
    for (int i = 0; i < client_number; i++){
        total_throughput += threads[i].get();
    } 
    return total_throughput;  
}


uint32_t insert_query_each_client(std::vector<std::vector<int32_t>> *total_points_stored, int shard_number, int client_number, int client_index, std::vector<std::string> server_ips, uint32_t points_to_operate, bool search_only)
{
    auto client = MDTrieClient(server_ips, shard_number);
    TimeStamp start, diff = 0; 
    bool finished_warmup = false;

    uint32_t op_count = 0;
    uint32_t effective_op_count = 0;
    std::vector<int32_t> max_values;
    std::vector<int32_t> min_values;

    if (current_dataset_idx == 1) {
        max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 58063825, 19980802};
        min_values = {1, 90000, 0, 0, 19920102, 19920131, 19920103, 81300, 19920101};
    }
    else if (current_dataset_idx == 2) {
        max_values = {7451541, 737170, 262926, 354850, 379379, 3097263, 703341, 8745, 20201206, 20201206};
        min_values = {1, 1, 0, 0, 0, 0, 0, 0, 20110211, 20110211};
    }
    else if (current_dataset_idx == 3) {
        max_values = {20160630, 20221220, 899, 898, 899, 898, 255, 198623000, 21474808, 1000, 1312,  3950589, 21474836, 138, 21474830};
        min_values = {20090101, 19700101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    }

    std::string line_range_search;
    std::string range_query_address;
    
    if (current_dataset_idx == 1)
        range_query_address = "/proj/trinity-PG0/Trinity/queries/tpch/tpch_query_converted";
    if (current_dataset_idx == 2)
        range_query_address = "/proj/trinity-PG0/Trinity/queries/github/github_query_new_converted";
    if (current_dataset_idx == 3)
        range_query_address = "/proj/trinity-PG0/Trinity/queries/nyc/nyc_query_new_converted";
    
    std::vector<std::string> line_list;
    std::ifstream file_range_search(range_query_address);

    while(std::getline(file_range_search, line_range_search)) {
        line_list.push_back(line_range_search);
    }

    // std::cout << "line_list: " << line_list.size() << std::endl;

    for (uint32_t point_idx = client_index; point_idx < points_to_operate; point_idx += client_number){

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

            // for (auto i : start_range) {
            //     std::cout << i << ", ";
            // }
            // std::cout << std::endl;

            // for (auto i : end_range) {
            //     std::cout << i << ", ";
            // }

            // std::cout << line_range_search;

            if (current_dataset_idx == 1)
                update_range_search_range_tpch(start_range, end_range, line_range_search);
            if (current_dataset_idx == 2)
                update_range_search_range_github(start_range, end_range, line_range_search);
            if (current_dataset_idx == 3)
                update_range_search_range_nyc(start_range, end_range, line_range_search);

            // for (auto i : start_range) {
            //     std::cout << i << ", ";
            // }
            // std::cout << std::endl;

            // for (auto i : end_range) {
            //     std::cout << i << ", ";
            // }
            // std::cout << std::endl;

            client.range_search_trie_send(start_range, end_range);
            client.range_search_trie_rec(found_points);

            // std::cout << found_points.size() << std::endl;
            // exit(-1);

            effective_op_count += found_points.size() - 1;
            found_points.clear();
        }
    }
    diff = GetTimestamp() - start;
    return ((float) effective_op_count / diff) * 1000000;
}

uint32_t total_client_insert_query(int shard_number, int client_number, std::vector<std::string> server_ips, int which_part = 0){

    std::string file_address;
    if (current_dataset_idx == 1)
        file_address = "/mntData/tpch_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 2)
        file_address = "/mntData/github_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 3)
        file_address = "/mntData/nyc_split_10/x" + std::to_string(which_part);

    points_to_skip = 5000000;
    if (current_dataset_idx == 2)
        points_to_skip = 50000000; // 50M

    uint32_t points_to_operate = points_to_skip + 500; 

    std::vector<std::vector<int32_t>> total_points_stored;

    if (current_dataset_idx == 1)
        total_points_stored = load_dataset_vector_tpch(file_address, points_to_operate);
    if (current_dataset_idx == 2) {
        total_points_stored = load_dataset_vector_github(file_address, points_to_operate);
    }
    if (current_dataset_idx == 3) {
        total_points_stored = load_dataset_vector_nyc(file_address, points_to_operate);
    }

    pkey_vect = primary_key_zipfan(points_to_operate);

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


    std::string file_address;
    if (current_dataset_idx == 1)
        file_address = "/mntData/tpch_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 2)
        file_address = "/mntData/github_split_10/x" + std::to_string(which_part);
    if (current_dataset_idx == 3)
        file_address = "/mntData/nyc_split_10/x" + std::to_string(which_part);

    std::vector<std::vector<int32_t>> total_points_stored;
    points_to_skip = 5000000;
    if (current_dataset_idx == 2)
        points_to_skip = 50000000; // 50M
    // points_to_skip = 5000000 / 10;

    uint32_t points_to_operate = points_to_skip + 500; 


    if (current_dataset_idx == 1)
        total_points_stored = load_dataset_vector_tpch(file_address, points_to_operate);
    if (current_dataset_idx == 2) {
        // total_points_stored = load_dataset_vector_github_query(file_address, points_to_operate);
        total_points_stored = load_dataset_vector_github(file_address, points_to_operate);
    }

    if (current_dataset_idx == 3) {
        total_points_stored = load_dataset_vector_nyc(file_address, points_to_operate);
    }

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
