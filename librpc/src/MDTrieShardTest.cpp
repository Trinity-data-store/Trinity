
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

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

const int DIMENSION = 6; 
n_leaves_t n_lines = 14252681;
const int BATCH_SIZE = 1024;
std::atomic<int> active_thread_num {0};
std::atomic<int> finished_thread_num {0};

std::atomic<TimeStamp> latency_start = 0;
std::atomic<TimeStamp> latency_diff = 0;

vector<vector <int32_t>> *get_data_vector(){

/** 
    Get data from the OSM dataset stored in a vector
*/

  FILE *fp = fopen("../libmdtrie/bench/data/osm_combined_updated.csv", "r");
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

  tqdm bar;
  n_leaves_t n_points = 0;
  auto data_vector = new vector<vector <int32_t>>;
  while ((read = getline(&line, &len, fp)) != -1)
  {
      bar.progress(n_points, n_lines);
      vector<int32_t> point;
      char *token = strtok(line, ",");
      char *ptr;
      for (dimension_t i = 0; i < 8; i++){

          if (i == 1){
              token = strtok(nullptr, ",");
              token = strtok(nullptr, ",");
          }
          token = strtok(nullptr, ",");
          if (i < 8 - DIMENSION)
              continue;

          point.push_back(strtoul(token, &ptr, 10));
      }
      data_vector->push_back(point);
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
  uint32_t warmup_cooldown_points = total_points_to_insert / 5;

  int sent_count = 0;
  uint32_t current_pos;
  // uint32_t start_pos_measurement = 0;
  // uint32_t end_pos_measurement = 0;

  TimeStamp start = 0; 
  TimeStamp diff = 0;

  for (current_pos = start_pos; current_pos <= end_pos; current_pos++){
    
    if (current_pos == start_pos + warmup_cooldown_points){
      start = GetTimestamp();
      // if (latency_start == 0)
      //   latency_start = start;
      // active_thread_num ++;
    }

    // if (start == 0 && active_thread_num == client_number){

    //   start = GetTimestamp();
    //   start_pos_measurement = current_pos;
    //   if (latency_start == 0)
    //     latency_start = start;
    // }

    if (current_pos == end_pos - warmup_cooldown_points){
      // finished_thread_num ++;
      diff = GetTimestamp() - start;
      // if (finished_thread_num == client_number)
      //   latency_diff = GetTimestamp() - latency_start;
    }

    // if (diff == 0 && start != 0 && finished_thread_num != 0){

    //   diff = GetTimestamp() - start;
    //   end_pos_measurement = current_pos;
    //   if (finished_thread_num == client_number)
    //     latency_diff = GetTimestamp() - latency_start;
    // }

    if (sent_count != 0 && sent_count % BATCH_SIZE == 0){
        for (uint32_t j = current_pos - sent_count; j < current_pos; j++){
            client.insert_rec(j);
        }
        sent_count = 0;
    }

    vector<int32_t> data_point = (*data_vector)[current_pos];
    client.insert_send(data_point, current_pos);
    sent_count ++;
  }

  for (uint32_t j = end_pos - sent_count + 1; j <= end_pos; j++){
      client.insert_rec(j);
  }

  return std::make_tuple(((float) (total_points_to_insert - 2 * warmup_cooldown_points) / diff) * 1000000, diff, total_points_to_insert - 2 * warmup_cooldown_points);
  // return std::make_tuple(((float) (end_pos_measurement - start_pos_measurement + 1) / diff) * 1000000, diff, end_pos_measurement - start_pos_measurement + 1);
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

int main(int argc, char *argv[]){
  
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

  vector<vector <int32_t>> *data_vector = get_data_vector();
 
/** 
    Insert all points from the OSM dataset
*/

  // TimeStamp diff = 0;
  // TimeStamp start = GetTimestamp();
  std::tuple<uint32_t, float> return_tuple = total_client_insert(data_vector, client_number);
  uint32_t throughput = std::get<0>(return_tuple);
  float latency = std::get<1>(return_tuple);
  // diff = GetTimestamp() - start;

  cout << "Insertion Throughput add thread (pt / seconds): " << throughput << endl;
  cout << "Total end to end latency / number of points (us): " << latency << endl;
  cout << "Inserted Points: " << client.get_count() << endl << endl;

// /** 
//     Range Search full range
// */

  auto start_range = vector <int32_t>(DIMENSION, 0);
  auto end_range = vector <int32_t>(DIMENSION, 1 << 31);
  
  std::vector<int32_t> return_vect;
  TimeStamp start;
  TimeStamp diff;
  diff = 0;
  start = GetTimestamp();
  client.range_search_trie(return_vect, start_range, end_range);
  diff = GetTimestamp() - start;

  cout << "number of points found: " << return_vect.size() << endl;
  cout << "total number of data points: " << data_vector->size() << endl;
  cout << "Range Search Throughput (pt / seconds): " << ((float) return_vect.size() / diff) * 1000000 << endl;

  return 0;
}