#ifndef TrinityBenchShared_H
#define TrinityBenchShared_H

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

// TODO: Double check BATCH_SIZE
int BATCH_SIZE = 4096 * 10;
int WARMUP_FACTOR = 5;

// Each client insert lines from a file
uint32_t insert_each_client_from_file(std::string file_address, int shard_number, int client_number, int client_index, std::vector<std::string> server_ips, uint32_t points_to_insert = 75000707){
  
  auto client = MDTrieClient(server_ips, shard_number);
  std::ifstream file(file_address);
  std::string line;

  uint32_t start_line = points_to_insert / client_number * client_index;  
  uint32_t end_line = points_to_insert / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_line = points_to_insert - 1;

  uint32_t total_points_to_insert = end_line - start_line + 1;
  uint32_t warmup_points = total_points_to_insert / WARMUP_FACTOR;

  int sent_count = 0;
  TimeStamp start, diff = 0; 

  for (uint32_t current_line = start_line; current_line <= end_line; current_line++){
    
    if (current_line % 1000000 == 0) {
      cerr << "current_line: " << current_line << "," << start_line << "," << end_line << endl;
    }
    std::getline(file, line);

    if (current_line == start_line + warmup_points){
      start = GetTimestamp();
    }

    if (sent_count != 0 && sent_count % BATCH_SIZE == 0){
        for (uint32_t j = current_line - sent_count; j < current_line; j++){
            client.insert_rec(j);
            if (j == end_line){
              diff = GetTimestamp() - start;
            }
        }
        sent_count = 0;
    }
    vector <int32_t> data_point = parse_line_tpch(line);

      
    client.insert_send(data_point, current_line);
    sent_count ++;
  }

  for (uint32_t j = end_line - sent_count + 1; j <= end_line; j++){
      client.insert_rec(j);
      if (j == end_line){
        diff = GetTimestamp() - start;
      }
  }
  return ((float) (total_points_to_insert - warmup_points) / diff) * 1000000;
}

uint32_t total_client_insert_split_tpch(int shard_number, int client_number, std::vector<std::string> server_ips, int which_part = 0){

  std::vector<std::future<uint32_t>> threads; 
  threads.reserve(client_number);

  int start_split = 0;
  int end_split = client_number - 1;
  
  if (which_part == 1) {
    start_split = 0;
    end_split = 19;
  }
  if (which_part == 2) {
    start_split = 20;
    end_split = 39;
  }
  if (which_part == 3) {
    start_split = 40;
    end_split = 59;
  }

  for (int i = start_split; i <= end_split; i++){

    char buff[100];
    snprintf(buff, sizeof(buff), "/mntData/tpch_split/x%d", i);
    std::string split_address = buff;

    uint32_t points_to_insert = 16666667 / 10;
    if (i == 59) {
      points_to_insert = 16666647 / 10;
    }
    threads.push_back(std::async(insert_each_client_from_file, split_address, shard_number, 1, 0, server_ips, points_to_insert));
  }  

  uint32_t total_throughput = 0;
  for (int i = 0; i < client_number; i++){
    total_throughput += threads[i].get();
  } 
  return total_throughput;  
}


/**
 * Lookup given primary keys
 */

uint32_t lookup_each_client(int shard_number, int client_number, int client_index, std::vector<std::string> server_ips){

  auto client = MDTrieClient(server_ips, shard_number);

  uint32_t start_pos = total_points_count / client_number * client_index;
  uint32_t end_pos = total_points_count / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_pos = total_points_count - 1;

  uint32_t total_points_to_lookup = end_pos - start_pos + 1;
  uint32_t warmup_cooldown_points = total_points_to_lookup / WARMUP_FACTOR;

  int sent_count = 0;
  TimeStamp start, diff = 0; 

  for (uint32_t current_pos = start_pos; current_pos <= end_pos; current_pos++){
    
    if (current_pos == start_pos + warmup_cooldown_points){
      start = GetTimestamp();
    }

    if (sent_count != 0 && sent_count % BATCH_SIZE == 0){
        for (uint32_t j = current_pos - sent_count; j < current_pos; j++){
            std::vector<int32_t> rec_vect;
            client.primary_key_lookup_rec(rec_vect, j);
            if (j == end_pos - warmup_cooldown_points){
              diff = GetTimestamp() - start;
            }
        }
        sent_count = 0;
    }

    client.primary_key_lookup_send(current_pos);
    sent_count ++;
  }

  for (uint32_t j = end_pos - sent_count + 1; j <= end_pos; j++){
      std::vector<int32_t> rec_vect;
      if (j == end_pos - warmup_cooldown_points){
        diff = GetTimestamp() - start;
      }
      client.primary_key_lookup_rec(rec_vect, j);
  }
  return ((float) (total_points_to_lookup - 2 * warmup_cooldown_points) / diff) * 1000000;
}

uint32_t total_client_lookup(int shard_number, int client_number, std::vector<std::string> server_ips, int which_part = 0){

  std::vector<std::future<uint32_t>> threads; 
  threads.reserve(client_number);

  int start_split = 0;
  int end_split = client_number - 1;
  total_points_count /= 50;

  if (which_part == 1) {
    start_split = 0;
    end_split = 19;
  }
  if (which_part == 2) {
    start_split = 20;
    end_split = 39;
  }
  if (which_part == 3) {
    start_split = 40;
    end_split = 59;
  }

  for (int i = start_split; i <= end_split; i++){

    threads.push_back(std::async(lookup_each_client, shard_number, client_number, i, server_ips));
  }  

  uint32_t total_throughput = 0;
  for (int i = 0; i < client_number; i++){
    total_throughput += threads[i].get();
  } 
  return total_throughput;  
}

#endif //TrinityBenchShared_H
