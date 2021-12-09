#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "MDTrieShardClient.h"
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

n_leaves_t n_lines = 152806265;
const int BATCH_SIZE = 4096;
std::atomic<int> active_thread_num {0};
std::atomic<int> finished_thread_num {0};

std::atomic<TimeStamp> latency_start = 0;
std::atomic<TimeStamp> latency_diff = 0;

vector<vector <int32_t>> *get_data_vector(){

/** 
    Get data from the OSM dataset stored in a vector
*/

  FILE *fp = fopen("../libmdtrie/bench/data/osm_us_northeast_long_lat.csv", "r");
  char *line = nullptr;
  size_t len = 0;
  ssize_t read;

  if (fp == nullptr)
  {
      fprintf(stderr, "file not found\n");
      exit(EXIT_FAILURE);
  }  

  tqdm bar;
  n_leaves_t n_points = 0;
  auto data_vector = new vector<vector <int32_t>>;
  while ((read = getline(&line, &len, fp)) != -1)
  {
      bar.progress(n_points, n_lines);
      vector<int32_t> point(DATA_DIMENSION, 0);
      char *token = strtok(line, ",");
      char *ptr;
      for (dimension_t i = 0; i < 8; i++){
          token = strtok(nullptr, ",");
          if (i < 8 - DATA_DIMENSION)
              continue;
          point[i - (8 - DATA_DIMENSION)] = strtoul(token, &ptr, 10);
      }
      data_vector->push_back(point);
      n_points ++;
  }  
  bar.finish();
  return data_vector;
}



vector<vector <int32_t>> *get_data_vector_osm(std::vector<int32_t> &max_values, std::vector<int32_t> &min_values){


  char *line = nullptr;
  size_t len = 0;
  ssize_t read;
  FILE *fp = fopen("/home/ziming/osm/osm_us_northeast_timestamp.csv", "r");
  if (fp == nullptr)
  {
      fprintf(stderr, "file not found\n");
      exit(EXIT_FAILURE);
  }

  n_leaves_t n_points = 0;
  n_leaves_t n_lines = 152806264;
  total_points_count = n_lines;
  auto data_vector = new vector<vector <int32_t>>;

  tqdm bar;
  read = getline(&line, &len, fp);

  while ((read = getline(&line, &len, fp)) != -1)
  {
      vector <int32_t> point(DATA_DIMENSION, 0);
      bar.progress(n_points, n_lines);
      char *token = strtok(line, ",");
      char *ptr;

      for (dimension_t i = 0; i < DATA_DIMENSION; i++){
          token = strtok(nullptr, ",");
          point[i] = strtoul(token, &ptr, 10);
      }

      for (dimension_t i = 0; i < DATA_DIMENSION; i++){
          
          if (n_points == 0){
              max_values[i] = point[i];
              min_values[i] = point[i];
          }
          else {
              if (point[i] > max_values[i]){
                  max_values[i] = point[i];
              }
              if (point[i] < min_values[i]){
                  min_values[i] = point[i];
              }
          }          
      }

      if (n_points == n_lines)
          break;

      data_vector->push_back(point);
      n_points ++;
  }
  bar.finish();
  return data_vector;
}


vector<vector <int32_t>> *get_data_vector_filesystem(std::vector<int32_t> &max_values, std::vector<int32_t> &min_values){


  char *line = nullptr;
  size_t len = 0;
  ssize_t read;
  FILE *fp = fopen("../libmdtrie/bench/data/sample_shuf.txt", "r");
  if (fp == nullptr)
  {
      fprintf(stderr, "file not found\n");
      exit(EXIT_FAILURE);
  }

  n_leaves_t n_points = 0;
  n_leaves_t n_lines = 14583357;
  total_points_count = n_lines;
  auto data_vector = new vector<vector <int32_t>>;

  tqdm bar;

  while ((read = getline(&line, &len, fp)) != -1)
  {
      vector <int32_t> point(DATA_DIMENSION, 0);
      bar.progress(n_points, n_lines);
      char *token = strtok(line, " ");
      char *ptr;

      for (uint8_t i = 1; i <= 2; i ++){
          token = strtok(nullptr, " ");
      }

      for (dimension_t i = 0; i < DATA_DIMENSION; i++){
          token = strtok(nullptr, " ");
          point[i] = strtoul(token, &ptr, 10);
      }

      for (dimension_t i = 0; i < DATA_DIMENSION; i++){
          
          if (n_points == 0){
              max_values[i] = point[i];
              min_values[i] = point[i];
          }
          else {
              if (point[i] > max_values[i]){
                  max_values[i] = point[i];
              }
              if (point[i] < min_values[i]){
                  min_values[i] = point[i];
              }
          }          
      }

      if (n_points == n_lines)
          break;

      data_vector->push_back(point);
      n_points ++;
  }
  bar.finish();
  return data_vector;
}


vector<vector <int32_t>> *get_data_vector_tpch(std::vector<int32_t> &max_values, std::vector<int32_t> &min_values){

/** 
    Get data from the OSM dataset stored in a vector
*/

  std::ifstream infile("/home/ziming/tpch-dbgen/tpch_20/orders_lineitem_merged_inner.csv");

  std::string line;
  std::getline(infile, line);

  tqdm bar;
  n_leaves_t n_points = 0;
  n_lines = 110418170;
  // n_lines = 50000581;
  auto data_vector = new vector<vector <int32_t>>;

  while (std::getline(infile, line))
  {
      bar.progress(n_points, n_lines);
      std::stringstream ss(line);
      vector <int32_t> point(DATA_DIMENSION, 0);

      // Parse string by ","
      int leaf_point_index = 0;
      int index = -1;

      // Kept indexes: 
      // [4, 5, 6, 7, 10, 11, 12, 16, 17]
      // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
      while (ss.good())
      {
          index ++;
          std::string substr;
          std::getline(ss, substr, ',');
      
          int32_t num;
          if (index == 5 || index == 6 || index == 7 || index == 16) // float with 2dp
          {
              num = static_cast<int32_t>(std::stof(substr) * 100);
          }
          else if (index == 10 || index == 11 || index == 12 || index == 17) //yy-mm-dd
          {
              substr.erase(std::remove(substr.begin(), substr.end(), '-'), substr.end());
              num = static_cast<int32_t>(std::stoul(substr));
          }
          else if (index == 8 || index == 9 || index == 13 || index == 15 || index == 18) //skip text
              continue;
          else if (index == 0 || index == 1 || index == 2 || index == 14) // secondary keys
              continue;
          else if (index == 19) // all 0
              continue;
          else if (index == 3) // lineitem
              continue;
          else
              num = static_cast<int32_t>(std::stoul(substr));

      
          point[leaf_point_index] = num;
          leaf_point_index++;
      }
      
      if (n_points == n_lines)
          break;

      data_vector->push_back(point);

      for (dimension_t i = 0; i < DATA_DIMENSION; i++){
          if (point[i] > max_values[i])
              max_values[i] = point[i];
          if (point[i] < min_values[i])
              min_values[i] = point[i];         
      }    
      n_points ++;
  }
  bar.finish();
  return data_vector;
}

std::tuple<uint32_t, uint32_t, uint32_t> insert_each_client(vector<vector <int32_t>> *data_vector, int client_number, int client_index){

  auto client = MDTrieClient();
  uint32_t start_pos = data_vector->size() / client_number * client_index;
  uint32_t end_pos = data_vector->size() / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_pos = data_vector->size() - 1;

  uint32_t total_points_to_insert = end_pos - start_pos + 1;
  uint32_t warmup_cooldown_points = total_points_to_insert / 3;

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
    if (current_pos == 19065010){
      raise(SIGINT);
    }
    client.insert_send(data_point, current_pos);
    sent_count ++;
  }

  for (uint32_t j = end_pos - sent_count + 1; j <= end_pos; j++){
      client.insert_rec(j);
      if (j == end_pos - warmup_cooldown_points){

        diff = GetTimestamp() - start;
      }
  }
  return std::make_tuple(((float) (total_points_to_insert - 2 * warmup_cooldown_points) / diff) * 1000000, diff, total_points_to_insert - 2 * warmup_cooldown_points);
}


void insert_for_join_table(vector<vector <int32_t>> *data_vector, int client_number, int client_index){

  // std::vector<std::string> server_ips = {"172.28.229.152", "172.28.229.153"};
  // std::vector<std::string> server_ips = {"172.28.229.152"};
  std::vector<std::string> server_ips = {"172.28.229.152", "172.28.229.153", "172.28.229.151", "172.28.229.149", "172.28.229.148"};

  // auto client = MDTrieClient(server_ips, 48);
  auto client = MDTrieClient(server_ips, 20);

  uint32_t start_pos = data_vector->size() / client_number * client_index;
  uint32_t end_pos = data_vector->size() / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_pos = data_vector->size() - 1;

  int sent_count = 0;
  uint32_t current_pos;

  for (current_pos = start_pos; current_pos <= end_pos; current_pos++){

    if (sent_count != 0 && sent_count % BATCH_SIZE == 0){
        for (uint32_t j = current_pos - sent_count; j < current_pos; j++){
            client.insert_rec(j);
        }
        sent_count = 0;
    }
    vector<int32_t> data_point = (*data_vector)[current_pos];
    client.insert_send(data_point, current_pos);
    if (current_pos % (data_vector->size() / 50) == 0)
      std::cout << "inserted: " << current_pos << std::endl;
    sent_count ++;
  }

  for (uint32_t j = end_pos - sent_count + 1; j <= end_pos; j++){
      client.insert_rec(j);
  }
}

std::tuple<uint32_t, float> total_client_insert(vector<vector <int32_t>> *data_vector, int client_number){

  std::vector<std::future<std::tuple<uint32_t, uint32_t, uint32_t>>> threads; 
  threads.reserve(client_number);

  for (int i = 0; i < client_number; i++){

    threads.push_back(std::async(insert_each_client, data_vector, client_number, i));
  }  

  uint32_t total_throughput = 0;
  uint32_t total_latency = 0;
  uint32_t total_points = 0;
  for (int i = 0; i < client_number; i++){

    std::tuple<uint32_t, uint32_t, uint32_t> return_tuple = threads[i].get();
    total_throughput += std::get<0>(return_tuple);
    total_latency += std::get<1>(return_tuple);    
    total_points += std::get<2>(return_tuple);

    if (i == 0)
      cout << "total throughput: [" << total_throughput << "] total_latency: [" << total_latency << "] total_points: [" << total_points << "]" << endl;
  } 

  return std::make_tuple(total_throughput, (float) total_latency / total_points);  
}


// LOOKUP Code

std::tuple<uint32_t, uint32_t, uint32_t> lookup_each_client(vector<vector <int32_t>> *data_vector, int client_number, int client_index){

  auto client = MDTrieClient();
  uint32_t start_pos = data_vector->size() / client_number * client_index;
  uint32_t end_pos = data_vector->size() / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_pos = data_vector->size() - 1;

  uint32_t total_points_to_lookup = end_pos - start_pos + 1;
  uint32_t warmup_cooldown_points = total_points_to_lookup / 3;

  int sent_count = 0;
  uint32_t current_pos;

  TimeStamp start = 0; 
  TimeStamp diff = 0;

  for (current_pos = start_pos; current_pos <= end_pos; current_pos++){
    
    if (current_pos == start_pos + warmup_cooldown_points){

      // cout << "started measurement: " << client_index << endl;
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

  // cout << "ended measurement: " << client_index << endl;

  for (uint32_t j = end_pos - sent_count + 1; j <= end_pos; j++){
      std::vector<int32_t> rec_vect;
      if (j == end_pos - warmup_cooldown_points){
        diff = GetTimestamp() - start;
      }
      client.primary_key_lookup_rec(rec_vect, j);
  }

  return std::make_tuple(((float) (total_points_to_lookup - 2 * warmup_cooldown_points) / diff) * 1000000, diff, total_points_to_lookup - 2 * warmup_cooldown_points);
}

std::tuple<uint32_t, float> total_client_lookup(vector<vector <int32_t>> *data_vector, int client_number){

  std::vector<std::future<std::tuple<uint32_t, uint32_t, uint32_t>>> threads; 
  threads.reserve(client_number);

  for (int i = 0; i < client_number; i++){

    threads.push_back(std::async(lookup_each_client, data_vector, client_number, i));
  }  

  uint32_t total_throughput = 0;
  uint32_t total_latency = 0;
  uint32_t total_points = 0;
  for (int i = 0; i < client_number; i++){

    std::tuple<uint32_t, uint32_t, uint32_t> return_tuple = threads[i].get();
    total_throughput += std::get<0>(return_tuple);
    total_latency += std::get<1>(return_tuple);    
    total_points += std::get<2>(return_tuple);

    // if (i == 0)
    //   cout << "total throughput: [" << total_throughput << "] total_latency: [" << total_latency << "] total_points: [" << total_points << "]" << endl;
  } 

  return std::make_tuple(total_throughput, (float) total_latency / total_points);  
}

std::tuple<uint32_t, uint32_t, uint32_t> range_search_each_client(vector<vector <int32_t>> *start_range_collection, vector<vector <int32_t>> *end_range_collection, int client_number, int client_index){

  auto client = MDTrieClient();
  uint32_t start_pos = start_range_collection->size() / client_number * client_index;
  uint32_t end_pos = start_range_collection->size() / client_number * (client_index + 1) - 1;

  if (client_index == client_number - 1)
    end_pos = start_range_collection->size() - 1;

  uint32_t total_points_to_lookup = end_pos - start_pos + 1;
  uint32_t warmup_cooldown_points = total_points_to_lookup / 7;
  uint32_t current_pos;
  int sent_count = 0;

  TimeStamp start = 0; 
  TimeStamp diff = 0;
  uint32_t total_points_returned = 0;

  for (current_pos = start_pos; current_pos <= end_pos; current_pos++){
    
    if (current_pos == start_pos + warmup_cooldown_points){
      start = GetTimestamp();
    }

    if (sent_count != 0 && sent_count % BATCH_SIZE == 0){
        for (uint32_t j = current_pos - sent_count; j < current_pos; j++){

          std::vector<int32_t> rec_vect;

          // TimeStamp start_here = GetTimestamp();
          client.range_search_trie_rec(rec_vect);
          // cout << "receive request time time: " << GetTimestamp() - start_here << " for index: " << j << " with points: " << rec_vect.size() << endl;

          if (j == end_pos - warmup_cooldown_points){
            diff = GetTimestamp() - start;
          }

          if (j >= start_pos + warmup_cooldown_points && j < end_pos - warmup_cooldown_points){
            total_points_returned += rec_vect.size();
          }
        }
        sent_count = 0;
    }

    // TimeStamp start_here = GetTimestamp();
    std::vector<int32_t> start_range = (* start_range_collection)[current_pos];
    std::vector<int32_t> end_range = (* end_range_collection)[current_pos];

    client.range_search_trie_send(start_range, end_range);

    // cout << "send a request time: " << GetTimestamp() - start_here << " for " << current_pos << endl;
    sent_count ++;

  }

  for (uint32_t j = end_pos - sent_count + 1; j <= end_pos; j++){
      std::vector<int32_t> rec_vect;
      client.range_search_trie_rec(rec_vect);

      if (j == end_pos - warmup_cooldown_points){
        diff = GetTimestamp() - start;
      }

      if (j >= start_pos + warmup_cooldown_points && j < end_pos - warmup_cooldown_points)
        total_points_returned += rec_vect.size();      
  }
  return std::make_tuple(((float) total_points_returned / diff) * 1000000, diff, total_points_to_lookup - 2 * warmup_cooldown_points);
}


std::tuple<uint32_t, float> total_client_range_search(vector<vector <int32_t>> *start_range_collection, vector<vector <int32_t>> *end_range_collection, int client_number){

  std::vector<std::future<std::tuple<uint32_t, uint32_t, uint32_t>>> threads; 
  threads.reserve(client_number);

  for (int i = 0; i < client_number; i++){

    threads.push_back(std::async(range_search_each_client, start_range_collection, end_range_collection, client_number, i));
  }  

  uint32_t total_throughput = 0;
  uint32_t total_latency = 0;
  uint32_t total_requests = 0;
  for (int i = 0; i < client_number; i++){

    std::tuple<uint32_t, uint32_t, uint32_t> return_tuple = threads[i].get();
    total_throughput += std::get<0>(return_tuple);
    total_latency += std::get<1>(return_tuple);    
    total_requests += std::get<2>(return_tuple);

    if (i == 0)
      cout << "total throughput: [" << total_throughput << "] total_latency: [" << total_latency << "] total_requests: [" << total_requests << "]" << endl;
  } 

  return std::make_tuple(total_throughput, (float) total_latency / total_requests);  
}



int main(int argc, char *argv[]){



  std::vector<std::string> server_ips = {"172.28.229.152", "172.28.229.153", "172.28.229.151", "172.28.229.149", "172.28.229.148"};
  // std::vector<std::string> server_ips = {"172.28.229.152", "172.28.229.153"};
  // std::vector<std::string> server_ips = {"172.28.229.152"};
  // auto client_join_table = MDTrieClient(server_ips, 48);
  auto client_osm = MDTrieClient(server_ips, 20);


  client_osm.ping();
  std::vector<int32_t> max_values(DATA_DIMENSION, 0);
  std::vector<int32_t> min_values(DATA_DIMENSION, 2147483647);

  vector<vector <int32_t>> *data_vector_osm = get_data_vector_osm(max_values, min_values);
  TimeStamp start, diff;

  start = GetTimestamp();
  insert_for_join_table(data_vector_osm, 1, 0);
  diff = GetTimestamp() - start;
  std::cout << "Insertion end-to-end latency: " << diff << std::endl;
  std::cout << "Storage Overhead" << client_osm.get_count()  << std::endl;
  

//  ********* OSM QUERY 1:


  std::vector<int32_t>start_range_join(DATA_DIMENSION, 0);
  std::vector<int32_t>end_range_join(DATA_DIMENSION, 0);

  for (dimension_t i = 0; i < 4; i++){
      start_range_join[i] = min_values[i];
      end_range_join[i] = max_values[i];

      if (i == 0){
          start_range_join[i] = 1;  
          end_range_join[i] = 2;
      }
      if (i == 1)
      {
          start_range_join[i] = 20200600;  
          end_range_join[i] = 20200700;
      }
  }
  std::vector<int32_t> found_points;
  start = GetTimestamp();
  client_osm.range_search_trie(found_points, start_range_join, end_range_join);
  diff = GetTimestamp() - start;

  std::cout << found_points.size() << std::endl;
  std::cout << "Range Search end to end latency 1: " << diff << std::endl;

//  ********* OSM QUERY 2:

  for (dimension_t i = 0; i < 4; i++){
      start_range_join[i] = min_values[i];
      end_range_join[i] = max_values[i];

      if (i == 2){
          start_range_join[i] = 715000010;  
          end_range_join[i] = 720000010;
      }
      if (i == 3)
      {
          start_range_join[i] = 419000000;  
          end_range_join[i] = 420000000;
      }
  }
  found_points.clear();
  start = GetTimestamp();
  client_osm.range_search_trie(found_points, start_range_join, end_range_join);
  diff = GetTimestamp() - start;

  std::cout << found_points.size() << std::endl;
  std::cout << "Range Search end to end latency 2: " << diff << std::endl;


//  ********* OSM QUERY 3:

  for (dimension_t i = 0; i < 4; i++){
      start_range_join[i] = min_values[i];
      end_range_join[i] = max_values[i];
      if (i == 1){
          start_range_join[i] = 20200000;
          end_range_join[i] = max_values[i];
      }
      if (i == 2){
          start_range_join[i] = 719500010;  
          end_range_join[i] = 720000010;
      }
      if (i == 3)
      {
          start_range_join[i] = 419500000;  
          end_range_join[i] = 420000000;
      }
  }
  found_points.clear();
  start = GetTimestamp();
  client_osm.range_search_trie(found_points, start_range_join, end_range_join);
  diff = GetTimestamp() - start;

  std::cout << found_points.size() << std::endl;
  std::cout << "Range Search end to end latency 2: " << diff << std::endl;

  exit(0);

  vector<vector <int32_t>> *data_vector_filesystem = get_data_vector_filesystem(max_values, min_values);
  auto client_join_table = MDTrieClient(server_ips, 1);

  start = GetTimestamp();
  insert_for_join_table(data_vector_filesystem, 1, 0);
  diff = GetTimestamp() - start;
  std::cout << "Insertion end-to-end latency: " << diff << std::endl;
  std::cout << "Storage Overhead" << client_join_table.get_count()  << std::endl;

  // std::vector<int32_t>start_range_join(DATA_DIMENSION, 0);
  // std::vector<int32_t>end_range_join(DATA_DIMENSION, 0);
  // [ "create_time,modify_time,access_time,change_time,owner_id,group_id"]
  // raise(SIGINT);
  for (dimension_t i = 0; i < 7; i++){
      start_range_join[i] = min_values[i];
      end_range_join[i] = max_values[i];

      if (i == 1){
          start_range_join[i] = 1399000000;  //EXTENDEDPRICE <= 100000
          end_range_join[i] = 1400000000;
      }
      if (i == 0)
      {
          start_range_join[i] = 1000000000;  // TOTALPRICE >= 50000 (2dp)
          end_range_join[i] = 1400000000;
      }
      if (i == 4){
          start_range_join[i] = 100;  // DISCOUNT >= 0.05
          end_range_join[i] = 100000;
      }
  }
  // std::vector<int32_t> found_points;
  found_points.clear();
  start = GetTimestamp();
  client_join_table.range_search_trie(found_points, start_range_join, end_range_join);
  diff = GetTimestamp() - start;

  std::cout << found_points.size() << std::endl;
  std::cout << "Range Search end to end latency 1: " << diff << std::endl;






  exit(0);

/** 
    Join table Test
*/  
/*
  std::vector<std::string> server_ips = {"172.28.229.152", "172.28.229.153", "172.28.229.151", "172.28.229.149", "172.28.229.148"};
  // std::vector<std::string> server_ips = {"172.28.229.152", "172.28.229.153"};
  // std::vector<std::string> server_ips = {"172.28.229.152"};
  auto client_join_table = MDTrieClient(server_ips, 48);

  client_join_table.ping();
  std::vector<int32_t> max_values(DATA_DIMENSION, 0);
  std::vector<int32_t> min_values(DATA_DIMENSION, 2147483647);
  vector<vector <int32_t>> *data_vector_join_table = get_data_vector_tpch(max_values, min_values);

  TimeStamp start, diff;

  
  start = GetTimestamp();
  insert_for_join_table(data_vector_join_table, 1, 0);
  diff = GetTimestamp() - start;
  std::cout << "Insertion end-to-end latency: " << diff << std::endl;
  std::cout << "Storage Overhead" << client_join_table.get_count()  << std::endl;

  // exit(0);

  std::vector<int32_t>start_range_join(DATA_DIMENSION, 0);
  std::vector<int32_t>end_range_join(DATA_DIMENSION, 0);

  // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
  for (dimension_t i = 0; i < DATA_DIMENSION; i++){
      start_range_join[i] = min_values[i];
      end_range_join[i] = max_values[i];

      if (i == 0)
          start_range_join[i] = 20;  //QUANTITY >= 20
      if (i == 7)
      {
          start_range_join[i] = 1000000;   // TOTALPRICE >= 10000 (2dp)
          end_range_join[i] = 5000000;  // TOTALPRICE <= 50000 (2dp)
      }
      if (i == 2)
          start_range_join[i] = 1;  // DISCOUNT >= 0.01
  }
  std::vector<int32_t> found_points;
  start = GetTimestamp();
  client_join_table.range_search_trie(found_points, start_range_join, end_range_join);
  diff = GetTimestamp() - start;

  std::cout << found_points.size() << std::endl;
  std::cout << "Range Search Latency 1: " << (float) diff / found_points.size() << std::endl;
  std::cout << "Range Search end to end latency 1: " << diff << std::endl;

  // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
  for (dimension_t i = 0; i < DATA_DIMENSION; i++){
      start_range_join[i] = min_values[i];
      end_range_join[i] = max_values[i];

      if (i == 1)
          end_range_join[i] = 10000000;  //EXTENDEDPRICE <= 100000
      if (i == 7)
      {
          start_range_join[i] = 5000000;  // TOTALPRICE >= 50000 (2dp)
      }
      if (i == 3)
          start_range_join[i] = 5;  // DISCOUNT >= 0.05
  }

  found_points.clear();
  start = GetTimestamp();
  client_join_table.range_search_trie(found_points, start_range_join, end_range_join);
  diff = GetTimestamp() - start;

  std::cout << found_points.size() << std::endl;
  std::cout << "Range Search Latency 2: " << (float) diff / found_points.size() << std::endl;
  std::cout << "Range Search end to end latency 2: " << diff << std::endl;


  // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
  for (dimension_t i = 0; i < DATA_DIMENSION; i++){
      start_range_join[i] = min_values[i];
      end_range_join[i] = max_values[i];

      if (i == 1)
          end_range_join[i] = 5000000;  //EXTENDEDPRICE <= 50000
      if (i == 7)
      {
          start_range_join[i] = 40000000;  // TOTALPRICE >= 400000 (2dp)
      }
      if (i == 3)
          start_range_join[i] = 5;  // DISCOUNT >= 0.05
  }

  // std::vector<int32_t> found_points;
  found_points.clear();
  start = GetTimestamp();
  client_join_table.range_search_trie(found_points, start_range_join, end_range_join);
  diff = GetTimestamp() - start;

  std::cout << found_points.size() << std::endl;
  std::cout << "Range Search Latency 3: " << (float) diff / found_points.size() << std::endl;
  std::cout << "Range Search end to end latency 3: " << diff << std::endl;
*/
  return 0;

 
  if (argc != 2 && argc != 3){
    cout << "wrong number of arguments" << endl;
    return 0;
  }

  int client_number;
  if (argc == 3){
    client_number = atoi(argv[2]);
  }

  client_number = atoi(argv[1]);

  cout << "client number: " << client_number << endl;
  auto client = MDTrieClient();
  client.ping();
  vector<vector <int32_t>> *data_vector = get_data_vector();
 
/** 
    Insert all points from the OSM dataset
*/

  std::tuple<uint32_t, float> return_tuple = total_client_insert(data_vector, client_number);
  uint32_t throughput = std::get<0>(return_tuple);
  float latency = std::get<1>(return_tuple);

  cout << "Insertion Throughput add thread (pt / seconds): " << throughput << endl;
  cout << "Latency (us): " << latency << endl;
  cout << "Inserted Points: " << client.get_count() << endl;

  return 0;

/**  Range Search Obtain Search Range

*/
  int32_t max_range = 1 << 31;
  auto start_range = vector <int32_t>(DATA_DIMENSION, 0);
  auto end_range = vector <int32_t>(DATA_DIMENSION, max_range);

  int32_t max[DATA_DIMENSION];
  int32_t min[DATA_DIMENSION];

  for (n_leaves_t itr = 0; itr < n_lines; itr++) {

      for (dimension_t i = 0; i < DATA_DIMENSION; i++) {

          if (itr == 1) {
              max[i] = (*data_vector)[itr][i];
              min[i] = (*data_vector)[itr][i];
          } else {
              if ((*data_vector)[itr][i] > max[i]) {
                  max[i] = (*data_vector)[itr][i];
              }
              if ((*data_vector)[itr][i] < min[i]) {
                  min[i] = (*data_vector)[itr][i];
              }
          }
      }
  }  

  // int itr = 10000;
  int i = 0;

  ofstream file("range_search_size_latency.csv", std::ios_base::app);
  vector<vector <int32_t>> start_range_collection;
  vector<vector <int32_t>> end_range_collection;

  while (start_range_collection.size() < 1000){
    
    if (i % 100 == 0)
      cout << "finished: " << i << endl;
      
    for (unsigned int j = 0; j < DATA_DIMENSION; j++){

      // start_range[j] = min[j] + (max[j] - min[j] + 1) / 10 * (rand() % 10);
      // end_range[j] = start_range[j] + (max[j] - start_range[j] + 1) / 10 * (rand() % 10);
      start_range[j] = min[j] + (max[j] - min[j] + 1) / 15 * (rand() % 15);
      end_range[j] = start_range[j] + (max[j] - start_range[j] + 1) / 15 * (rand() % 15);
    }

    std::vector<int32_t> return_vect;
    TimeStamp start = GetTimestamp();
    client.range_search_trie(return_vect, start_range, end_range);
    TimeStamp diff = GetTimestamp() - start;

    // if (return_vect.size() <= 1000 && return_vect.size() > 0){
    if (return_vect.size() > 0){
      cout << "found: " << i << endl;

      start_range_collection.push_back(start_range);
      end_range_collection.push_back(end_range);

      // for (int j = 0; j < DIMENSION; j++){
      //   file << start_range[j] << ",";
      // }
      // for (int j = 0; j < DIMENSION; j++){
      //   file << end_range[j] << ",";
      // }

      file << return_vect.size() << "," << diff;
      file << endl;      
      // return 0;
    }
    i++;
  }

  file.close();

  return 0;



/** 
     Range Search 


  FILE *fp = fopen("./range_search.csv", "r");

  char *line = nullptr;
  size_t len = 0;
  ssize_t read;

  if (fp == nullptr)
  {
      fprintf(stderr, "file not found\n");
      char cwd[PATH_MAX];
      if (getcwd(cwd, sizeof(cwd)) != nullptr) {
          printf("Current working dir: %s\n", cwd);
      } else {
          perror("getcwd() error");
      }
      exit(EXIT_FAILURE);
  }  

  vector<vector <int32_t>> start_range_collection;
  vector<vector <int32_t>> end_range_collection;

  while ((read = getline(&line, &len, fp)) != -1)
  {
      char *token = strtok(line, ",");

      char *ptr;
      vector <int32_t> start_range;
      vector <int32_t> end_range;
      start_range.push_back(strtoul(token, &ptr, 10));

      for (dimension_t i = 1; i < DIMENSION; i++){

        token = strtok(nullptr, ",");
        start_range.push_back(strtoul(token, &ptr, 10));
      }  

      for (dimension_t i = 0; i < DIMENSION; i++){

        token = strtok(nullptr, ",");
        end_range.push_back(strtoul(token, &ptr, 10));
      }  
      start_range_collection.push_back(start_range);
      end_range_collection.push_back(end_range);

      if (start_range_collection.size() == 200)
        break;
  }

  return_tuple = total_client_range_search(&start_range_collection, &end_range_collection, client_number);
  return_tuple = total_client_range_search(&start_range_collection, &end_range_collection, 1);

  throughput = std::get<0>(return_tuple);
  latency = std::get<1>(return_tuple);

  cout << "Range Search Throughput add thread (pt / seconds): " << throughput << endl;
  cout << "Latency (us): " << latency << endl;

  return 0;

*/

/** 
    Point Lookup from the OSM dataset
*/

  return_tuple = total_client_lookup(data_vector, client_number);
  throughput = std::get<0>(return_tuple);
  latency = std::get<1>(return_tuple);

  cout << "Primary Key Lookup Throughput add thread (pt / seconds): " << throughput << endl;
  cout << "Latency (us): " << latency << endl;

  return 0;

}