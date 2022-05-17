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

int BATCH_SIZE = 4096 * 4;
int WARMUP_FACTOR = 5;

// Go to a specific line of a file (0-indexed)
void GotoLine(std::ifstream& file, int num){

    file.seekg(std::ios::beg);
    for(int i=0; i < num; ++i){
      file.ignore(numeric_limits<streamsize>::max(), file.widen('\n'));
    }
}

uint32_t insert_each_client_mmap(char *map, int shard_number, int client_number, int client_index, std::vector<std::string> server_ips) {

  auto client = MDTrieClient(server_ips, shard_number);
  // get start_file_pos, end_file_pos, 0-indexed
  uint32_t start_line = total_points_count / client_number * client_index;  
  uint32_t end_line = total_points_count / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_line = total_points_count - 1;

  uint32_t total_points_to_insert = end_line - start_line + 1;
  uint32_t warmup_cooldown_points = total_points_to_insert / WARMUP_FACTOR;
  TimeStamp start, diff = 0; 
  int sent_count = 0;

  uint32_t line_num = 0;
  long map_index = 0;
  while (line_num < start_line) {
    if (map[map_index] == '\n') {
      line_num ++;
    }
    map_index ++;
  }
  char* str_start = map + map_index;
  char* str_end;
  vector <int32_t> data_point;

  while (line_num < end_line) {

    if (map[map_index] == '\n') {
      
      if (line_num % 1000000 == 0) {
        cerr << "current_line: " << line_num << "," << start_line << "," << end_line << endl;
      }
      str_end = map + map_index;
      line_num ++;
      {
        if (line_num == start_line + warmup_cooldown_points){
          start = GetTimestamp();
        }
        if (sent_count != 0 && sent_count % BATCH_SIZE == 0){
            for (uint32_t j = line_num - sent_count; j < line_num; j++){
                client.insert_rec(j);
                if (j == end_line - warmup_cooldown_points){
                  diff = GetTimestamp() - start;
                }
            }
            sent_count = 0;
        }
        // vector <int32_t> data_point = parse_line_tpch(line);
        // cerr << str_start - map << " " << str_end - map << endl;
        data_point = parse_line_tpch_mmap(map, str_start - map, str_end - map);
        client.insert_send(data_point, line_num);
        sent_count ++;
      }
      str_start = map + map_index + 1;
    }
    map_index ++;
  }

  for (uint32_t j = end_line - sent_count + 1; j <= end_line; j++){
      client.insert_rec(j);
      if (j == end_line - warmup_cooldown_points){
        diff = GetTimestamp() - start;
      }
  }
  client.push_global_cache();
  return ((float) (total_points_to_insert - 2 * warmup_cooldown_points) / diff) * 1000000;

}

// Each client insert lines from a file
uint32_t insert_each_client_from_file(const char *file_address, int shard_number, int client_number, int client_index, std::vector<std::string> server_ips){

  auto client = MDTrieClient(server_ips, shard_number);
  std::ifstream file(file_address);
  std::string line;

  // get start_file_pos, end_file_pos, 0-indexed
  uint32_t start_line = total_points_count / client_number * client_index;  
  uint32_t end_line = total_points_count / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_line = total_points_count - 1;

  uint32_t total_points_to_insert = end_line - start_line + 1;
  uint32_t warmup_cooldown_points = total_points_to_insert / WARMUP_FACTOR;

  int sent_count = 0;
  TimeStamp start, diff = 0; 

  GotoLine(file, start_line);
  for (uint32_t current_line = start_line; current_line <= end_line; current_line++){
    
    if (current_line % 1000000 == 0) {
      cerr << "current_line: " << current_line << "," << start_line << "," << end_line << endl;
    }
    std::getline(file, line);

    // continue;
    if (current_line == start_line + warmup_cooldown_points){
      start = GetTimestamp();
    }

    if (sent_count != 0 && sent_count % BATCH_SIZE == 0){
        for (uint32_t j = current_line - sent_count; j < current_line; j++){
            client.insert_rec(j);
            if (j == end_line - warmup_cooldown_points){
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
      if (j == end_line - warmup_cooldown_points){
        diff = GetTimestamp() - start;
      }
  }
  client.push_global_cache();
  return ((float) (total_points_to_insert - 2 * warmup_cooldown_points) / diff) * 1000000;
}

uint32_t total_client_insert(const char *file_address, int shard_number, int client_number, std::vector<std::string> server_ips){

  std::vector<std::future<uint32_t>> threads; 
  threads.reserve(client_number);

  for (int i = 0; i < client_number; i++){

    threads.push_back(std::async(insert_each_client_from_file, file_address, shard_number, client_number, i, server_ips));
  }  

  uint32_t total_throughput = 0;
  for (int i = 0; i < client_number; i++){
    total_throughput += threads[i].get();
  } 
  return total_throughput;  
}

uint32_t total_client_insert_mmap(char *map, int shard_number, int client_number, std::vector<std::string> server_ips){

  std::vector<std::future<uint32_t>> threads; 
  threads.reserve(client_number);

  for (int i = 0; i < client_number; i++){
    threads.push_back(std::async(insert_each_client_mmap, map, shard_number, client_number, i, server_ips));
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

uint32_t total_client_lookup(int shard_number, int client_number, std::vector<std::string> server_ips){

  std::vector<std::future<uint32_t>> threads; 
  threads.reserve(client_number);

  for (int i = 0; i < client_number; i++){
    threads.push_back(std::async(lookup_each_client, shard_number, client_number, i, server_ips));
  }  

  uint32_t total_throughput = 0;
  for (int i = 0; i < client_number; i++){
    total_throughput += threads[i].get();
  } 
  return total_throughput;  
}



uint32_t insert_each_client_old(vector<vector <int32_t>> *data_vector, int shard_number, int client_number, int client_index, std::vector<std::string> server_ips){

  auto client = MDTrieClient(server_ips, shard_number);
  uint32_t start_pos = data_vector->size() / client_number * client_index;
  uint32_t end_pos = data_vector->size() / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_pos = data_vector->size() - 1;

  uint32_t total_points_to_insert = end_pos - start_pos + 1;
  uint32_t warmup_cooldown_points = total_points_to_insert / WARMUP_FACTOR;

  int sent_count = 0;

  TimeStamp start, diff = 0; 

  for (uint32_t current_pos = start_pos; current_pos <= end_pos; current_pos++){
    
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

  client.push_global_cache();
  return ((float) (total_points_to_insert - 2 * warmup_cooldown_points) / diff) * 1000000;
}

uint32_t total_client_insert_old(vector<vector <int32_t>> *data_vector, int shard_number, int client_number, std::vector<std::string> server_ips){

  std::vector<std::future<uint32_t>> threads; 
  threads.reserve(client_number);

  for (int i = 0; i < client_number; i++){

    threads.push_back(std::async(insert_each_client_old, data_vector, shard_number, client_number, i, server_ips));
  }  

  uint32_t total_throughput = 0;
  for (int i = 0; i < client_number; i++){
    total_throughput += threads[i].get();
  } 
  return total_throughput;  
}


#endif //TrinityBenchShared_H
