#ifndef TrinityBenchShared_H
#define TrinityBenchShared_H

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

// TODO: Double check BATCH_SIZE
int BATCH_SIZE = 1024;
int WARMUP_FACTOR = 5;

struct throughput_latency {
  float throughput;
  float insertion_latency;
  float the_other_latency;
};

uint32_t insert_each_client_sync(std::string file_address, int shard_number, int client_number, int client_index, std::vector<std::string> server_ips, uint32_t points_to_insert = 75000707){
  
  auto client = MDTrieClient(server_ips, shard_number);
  std::ifstream file(file_address);
  std::string line;

  uint32_t start_line = points_to_insert / client_number * client_index;  
  uint32_t end_line = points_to_insert / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_line = points_to_insert - 1;

  uint32_t total_points_to_insert = end_line - start_line + 1;
  uint32_t warmup_points = total_points_to_insert / WARMUP_FACTOR;

  TimeStamp start, diff = 0; 

  for (uint32_t current_line = start_line; current_line <= end_line; current_line++){
    
    if (current_line % 200000 == 0) {
      cerr << "current_line: " << current_line << "," << start_line << "," << end_line << endl;
    }
    std::getline(file, line);

    if (current_line == start_line + warmup_points){
      start = GetTimestamp();
    }

    vector <int32_t> data_point = parse_line_tpch(line);

    client.insert(data_point, current_line);
  }
  diff = GetTimestamp() - start;
  return ((float) (total_points_to_insert - warmup_points) / diff) * 1000000;
}

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

  int start_split = which_part * client_number;
  int end_split = which_part * client_number + client_number;
  
  /*
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
    // end_split = 59;
    end_split = 59;
  }
  */

  for (int i = start_split; i < end_split; i++){

    char buff[100];
    snprintf(buff, sizeof(buff), "/mntData/tpch_split_600/x%d", i);
    std::string split_address = buff;
    uint32_t points_to_insert = 1666667;
    if (i == 599) {
      points_to_insert = 1666467;
    }
    // threads.push_back(std::async(insert_each_client_from_file, split_address, shard_number, 1, 0, server_ips, points_to_insert));
    threads.push_back(std::async(insert_each_client_sync, split_address, shard_number, 1, 0, server_ips, points_to_insert));

  }  
  uint32_t total_throughput = 0;
  for (int i = 0; i < end_split - start_split; i++){
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

    threads.push_back(std::async(lookup_each_client, shard_number, 1, 0, server_ips));
  }  

  uint32_t total_throughput = 0;
  for (int i = 0; i < client_number; i++){
    total_throughput += threads[i].get();
  } 
  return total_throughput;  
}

struct throughput_latency insert_lookup_each_client_batch(std::string file_address, int shard_number, int client_number, int client_index, std::vector<std::string> server_ips, uint32_t operations = 200){

  auto client = MDTrieClient(server_ips, shard_number);
  std::ifstream file_insertion(file_address);

  std::string line_insert;

  uint32_t start_pos = operations / client_number * client_index + client_index;
  uint32_t end_pos = operations / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_pos = operations - 1;


  TimeStamp start, diff, insertion_latency, lookup_latency = 0; 
  uint32_t insertion_count, lookup_count = 0;
  start = GetTimestamp();
  int num_for_loops = 0;

  for (uint32_t current_pos = start_pos; current_pos <= end_pos; current_pos++){
    num_for_loops ++;
    if (current_pos % (operations / 20) == 0)
      std::cout << "current_pos: " << current_pos << std::endl;

    bool do_insert;
    if (num_for_loops % 2 == 1)
      do_insert = false;
    else
      do_insert = true;

    std::getline(file_insertion, line_insert);

    if (do_insert)
    {
      vector <int32_t> data_point = parse_line_tpch(line_insert);
      TimeStamp s = GetTimestamp();
      client.insert_send(data_point, current_pos);
      client.insert_rec(current_pos);
      insertion_latency += GetTimestamp() - s;
      insertion_count ++;
    } else {

      TimeStamp s = GetTimestamp();
      const int send_batch = 19;
      for (int itr = 0; itr < send_batch; itr++) {
        client.primary_key_lookup_send(current_pos + itr);
      }

      for (int itr = 0; itr < send_batch; itr++) {
        std::vector<int32_t> rec_vect;
        client.primary_key_lookup_rec(rec_vect, current_pos + itr);
      }
      current_pos += send_batch - 1;

      lookup_latency += GetTimestamp() - s;
      lookup_count += send_batch;
    }
  }
  diff = GetTimestamp() - start;

  struct throughput_latency ret;
  ret.throughput = ((float) (insertion_count + lookup_count) / diff) * 1000000;
  ret.insertion_latency = ((float)insertion_latency / insertion_count) / 1000;
  ret.the_other_latency = ((float)lookup_latency / lookup_count) / 1000;
  return ret;
}

struct throughput_latency insert_lookup_each_client(std::string file_address, int shard_number, int client_number, int client_index, std::vector<std::string> server_ips, uint32_t operations = 200){

  auto client = MDTrieClient(server_ips, shard_number);
  std::ifstream file_insertion(file_address);

  std::string line_insert;

  uint32_t start_pos = operations / client_number * client_index + client_index;
  uint32_t end_pos = operations / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_pos = operations - 1;


  TimeStamp start, diff, insertion_latency, lookup_latency = 0; 
  uint32_t insertion_count, lookup_count = 0;
  start = GetTimestamp();

  for (uint32_t current_pos = start_pos; current_pos <= end_pos; current_pos++){
    
    if (current_pos % (operations / 20) == 0)
      std::cout << "current_pos: " << current_pos << std::endl;

    bool do_insert;
    if (current_pos % 20 != 19)
      do_insert = false;
    else
      do_insert = true;

    std::getline(file_insertion, line_insert);

    if (do_insert)
    {
      /*
        INSERTION
      **/

      vector <int32_t> data_point = parse_line_tpch(line_insert);
      TimeStamp s = GetTimestamp();
      client.insert_send(data_point, current_pos);
      client.insert_rec(current_pos);
      insertion_latency += GetTimestamp() - s;
      insertion_count ++;
    } else {

      TimeStamp s = GetTimestamp();
      client.primary_key_lookup_send(current_pos);
      std::vector<int32_t> rec_vect;
      client.primary_key_lookup_rec(rec_vect, current_pos);
      lookup_latency += GetTimestamp() - s;
      lookup_count ++;
      /*
        RANGE SEARCH
      **/

    }
  }
  diff = GetTimestamp() - start;
  struct throughput_latency ret;
  ret.throughput = ((float) (insertion_count + lookup_count) / diff) * 1000000;
  ret.insertion_latency = ((float)insertion_latency / insertion_count) / 1000;
  ret.the_other_latency = ((float)lookup_latency / lookup_count) / 1000;
  return ret;
}

struct throughput_latency insert_query_each_client(std::string file_address, std::string range_query_address, int shard_number, int client_number, int client_index, std::vector<std::string> server_ips, uint32_t operations = 200){

  auto client = MDTrieClient(server_ips, shard_number);
  std::ifstream file_insertion(file_address);
  std::ifstream file_range_search(range_query_address);
  std::string line_insert;
  std::string line_range_search;
  // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
  std::vector<int32_t> max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 58063825, 19980802};
  std::vector<int32_t> min_values = {1, 90000, 0, 0, 19920102, 19920131, 19920103, 81300, 19920101};

  uint32_t start_pos = operations / client_number * client_index + client_index;
  uint32_t end_pos = operations / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_pos = operations - 1;

  TimeStamp start = 0;
  TimeStamp diff = 0;
  TimeStamp insertion_latency = 0;
  TimeStamp query_latency = 0; 
  uint32_t insertion_count = 0;
  uint32_t query_count = 0;
  start = GetTimestamp();

  // std::cout << "insertion_count: " << insertion_count << std::endl;
  // std::cout << "insertion_latency: " << insertion_latency << std::endl;
  // std::cout << "query_count: " << query_count << std::endl;
  // std::cout << "query_latency " << query_latency << std::endl;

  for (uint32_t current_pos = start_pos; current_pos <= end_pos; current_pos++){
    

    bool do_insert;
    if (current_pos % 20 != 19)
      do_insert = false;
    else
      do_insert = true;

    std::getline(file_insertion, line_insert);
    std::getline(file_range_search, line_range_search);

    if (current_pos % (operations / 20) == 0) {
      std::cout << "current_pos: " << current_pos << std::endl;
      // std::cout << line_insert << std::endl;
      // std::cout << line_range_search << std::endl;
    }


    if (do_insert)
    {
      /*
        INSERTION
      **/

      vector <int32_t> data_point = parse_line_tpch(line_insert);
      TimeStamp s = GetTimestamp();
      client.insert_send(data_point, current_pos);
      client.insert_rec(current_pos);
      insertion_latency += GetTimestamp() - s;
      insertion_count ++;
    } else {

      /*
        RANGE SEARCH
      **/

      std::vector<int32_t> found_points;
      std::vector<int32_t> start_range = min_values;
      std::vector<int32_t> end_range = max_values;

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
      TimeStamp s = GetTimestamp();
      // client.range_search_trie(found_points, start_range, end_range);
      client.range_search_trie_send(start_range, end_range);
      client.range_search_trie_rec(found_points);

      query_latency += GetTimestamp() - s;

      query_count ++;
      found_points.clear();
    }
  }
  diff = GetTimestamp() - start;
  struct throughput_latency ret;
  ret.throughput = ((float) (insertion_count + query_count) / diff) * 1000000;
  /*
  std::cout << "insertion_count: " << insertion_count << std::endl;
  std::cout << "insertion_latency: " << insertion_latency << std::endl;
  std::cout << "query_count: " << query_count << std::endl;
  std::cout << "query_latency " << query_latency << std::endl;
  */
  ret.insertion_latency = ((float)insertion_latency / insertion_count) / 1000;
  ret.the_other_latency = ((float)query_latency / query_count) / 1000;
  return ret;
}


struct throughput_latency total_client_insert_query(int shard_number, int client_number, std::vector<std::string> server_ips, int which_part = 0){

  std::vector<std::future<struct throughput_latency>> threads; 

  int start_split = 0;
  int end_split = client_number - 1;
  total_points_count /= 100;

  if (which_part == 1) {
    start_split = 55;
    end_split = 59;
  }
  if (which_part == 2) {
    start_split = 20;
    end_split = 39;
  }
  if (which_part == 3) {
    start_split = 40;
    end_split = 59;
  }

  threads.reserve(end_split - start_split + 1);

  for (int i = start_split; i <= end_split; i++){
    char buff[100];
    snprintf(buff, sizeof(buff), "/mntData/tpch_split/x%d", i);
    std::string file_address = buff;
    std::string range_query_address = "../queries/tpch/tpch_query_new_converted";
    threads.push_back(std::async(insert_query_each_client, file_address, range_query_address, shard_number, 1, 0, server_ips, 200));
  }  

  struct throughput_latency individual_ret;
  struct throughput_latency total_ret;
  total_ret.throughput = 0;
  total_ret.insertion_latency = 0;
  total_ret.the_other_latency = 0;
  for (int i = 0; i <  end_split - start_split + 1; i++){
    individual_ret = threads[i].get();
    total_ret.throughput += individual_ret.throughput;
    // std::cout << "individual_ret.insertion_latency: " << individual_ret.insertion_latency << std::endl;
    // std::cout << "total_ret.insertion_latency: " << total_ret.insertion_latency << std::endl;
    total_ret.insertion_latency += individual_ret.insertion_latency;
    total_ret.the_other_latency += individual_ret.the_other_latency;
  } 
  total_ret.insertion_latency /=  end_split - start_split + 1;
  total_ret.the_other_latency /=  end_split - start_split + 1;
  // std::cout << "FINAL total_ret.insertion_latency: " << total_ret.insertion_latency << std::endl;

  return total_ret;  
}


struct throughput_latency total_client_insert_lookup(int shard_number, int client_number, std::vector<std::string> server_ips, int which_part = 0){

  std::vector<std::future<struct throughput_latency>> threads; 

  int start_split = 0;
  int end_split = client_number - 1;
  total_points_count /= 100;

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
  std::cout << "which_part: " << which_part << ", start_split: " << start_split << ", end_split: " << end_split << std::endl;
  threads.reserve(end_split - start_split + 1);

  for (int i = start_split; i <= end_split; i++){
    char buff[100];
    snprintf(buff, sizeof(buff), "/mntData/tpch_split/x%d", i);
    std::string file_address = buff;
    threads.push_back(std::async(insert_lookup_each_client, file_address, shard_number, 1, 0, server_ips, 100000));
  }  

  struct throughput_latency individual_ret;
  struct throughput_latency total_ret;
  total_ret.throughput = 0;
  total_ret.insertion_latency = 0;
  total_ret.the_other_latency = 0;

  for (int i = 0; i < end_split - start_split + 1; i++){
    individual_ret = threads[i].get();
    total_ret.throughput += individual_ret.throughput;
    total_ret.insertion_latency += individual_ret.insertion_latency;
    total_ret.the_other_latency += individual_ret.the_other_latency;
  } 
  total_ret.insertion_latency /=  end_split - start_split + 1;
  total_ret.the_other_latency /=  end_split - start_split + 1;
  return total_ret;  
}


#endif //TrinityBenchShared_H
