#ifndef TrinityBenchShared_H
#define TrinityBenchShared_H

#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "trie.h"
#include <tqdm.h>
#include <future>
#include <atomic>
#include <tuple>
#include <iostream>
#include <fstream>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

int BATCH_SIZE = 1024;
int WARMUP_FACTOR = 10;

/**
 * Insertion
 */

uint32_t insert_each_client(vector<vector <int32_t>> *data_vector, int client_number, int client_index, std::vector<std::string> server_ips){

  auto client = MDTrieClient(server_ips, client_number);
  uint32_t start_pos = data_vector->size() / client_number * client_index;
  uint32_t end_pos = data_vector->size() / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_pos = data_vector->size() - 1;

  uint32_t total_points_to_insert = end_pos - start_pos + 1;
  uint32_t warmup_cooldown_points = total_points_to_insert / WARMUP_FACTOR;

  int sent_count = 0;
  uint32_t current_pos;

  TimeStamp start = 0; 
  TimeStamp diff = 0;

  for (current_pos = start_pos; current_pos <= end_pos; current_pos++){
    
    if (current_pos == start_pos + warmup_cooldown_points){
      start = GetTimestamp();
    }
    if (sent_count != 0 && sent_count % BATCH_SIZE == 0){
        for (uint32_t j = current_pos - sent_count; j < current_pos; j++){
            client.insert_rec(j);
            if (j == end_pos - warmup_cooldown_points){
              diff = GetTimestamp() - start;
            }
        }
        sent_count = 0;
    }
    vector<int32_t> data_point = (*data_vector)[current_pos];
    client.insert_send(data_point, current_pos);
    sent_count ++;
  }

  for (uint32_t j = end_pos - sent_count + 1; j <= end_pos; j++){
      client.insert_rec(j);
      if (j == end_pos - warmup_cooldown_points){
        diff = GetTimestamp() - start;
      }
  }
  return ((float) (total_points_to_insert - 2 * warmup_cooldown_points) / diff) * 1000000;
}

uint32_t total_client_insert(vector<vector <int32_t>> *data_vector, int client_number, std::vector<std::string> server_ips){

  std::vector<std::future<uint32_t>> threads; 
  threads.reserve(client_number);

  for (int i = 0; i < client_number; i++){

    threads.push_back(std::async(insert_each_client, data_vector, client_number, i, server_ips));
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

uint32_t lookup_each_client(vector<vector <int32_t>> *data_vector, int client_number, int client_index, std::vector<std::string> server_ips){

  auto client = MDTrieClient(server_ips, client_number);

  uint32_t start_pos = data_vector->size() / client_number * client_index;
  uint32_t end_pos = data_vector->size() / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_pos = data_vector->size() - 1;

  uint32_t total_points_to_lookup = end_pos - start_pos + 1;
  uint32_t warmup_cooldown_points = total_points_to_lookup / WARMUP_FACTOR;

  int sent_count = 0;
  uint32_t current_pos;

  TimeStamp start = 0; 
  TimeStamp diff = 0;

  for (current_pos = start_pos; current_pos <= end_pos; current_pos++){
    
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

    vector<int32_t> data_point = (*data_vector)[current_pos];
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

uint32_t total_client_lookup(vector<vector <int32_t>> *data_vector, int client_number, std::vector<std::string> server_ips){

  std::vector<std::future<uint32_t>> threads; 
  threads.reserve(client_number);

  for (int i = 0; i < client_number; i++){
    threads.push_back(std::async(lookup_each_client, data_vector, client_number, i, server_ips));
  }  

  uint32_t total_throughput = 0;
  for (int i = 0; i < client_number; i++){
    total_throughput += threads[i].get();
  } 
  return total_throughput;  
}

#endif //TrinityBenchShared_H
