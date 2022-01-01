#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "MDTrieShardClient.h"
#include "TrinityBenchShared.h"
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
const int DIMENSION = 7;

vector<vector <int32_t>> *get_data_vector(std::vector<int32_t> &max_values, std::vector<int32_t> &min_values){

/** 
    Get data from the File System dataset stored in a vector
*/

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
      vector <int32_t> point(DIMENSION, 0);
      bar.progress(n_points, n_lines);
      char *token = strtok(line, " ");
      char *ptr;

      for (uint8_t i = 1; i <= 2; i ++){
          token = strtok(nullptr, " ");
      }

      for (dimension_t i = 0; i < DIMENSION; i++){
          token = strtok(nullptr, " ");
          point[i] = strtoul(token, &ptr, 10);
      }

      for (dimension_t i = 0; i < DIMENSION; i++){
          
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


int main(){

    std::vector<std::string> server_ips = {"172.28.229.152", "172.28.229.153", "172.28.229.151", "172.28.229.149", "172.29.249.30"};

    int shard_num = 48;
    int client_num = 128;
    auto client = MDTrieClient(server_ips, shard_num);
    if (!client.ping(0)){
        std::cerr << "Server setting wrong!" << std::endl;
        exit(-1);
    }

    TimeStamp start, diff;

    /** 
        Insert all points
    */

    std::tuple<uint32_t, float> return_tuple;
    uint32_t throughput;

    std::vector<int32_t> max_values(DIMENSION, 0);
    std::vector<int32_t> min_values(DIMENSION, 2147483647);
    vector<vector <int32_t>> *data_vector = get_data_vector(max_values, min_values);

    start = GetTimestamp();
    throughput = total_client_insert(data_vector, client_num, server_ips);
    diff = GetTimestamp() - start;

    cout << "Insertion Throughput (pt / seconds): " << throughput << endl;
    cout << "End-to-end Latency (us): " << diff << endl;
    cout << "Storage: " << client.get_size() << endl;

    /**   
        Point Lookup given primary key
    */

    start = GetTimestamp();
    throughput = total_client_lookup(data_vector, client_num, server_ips);

    diff = GetTimestamp() - start;
    cout << "Primary Key Lookup Throughput (pt / seconds): " << throughput << endl;

    /**   
     * Sample Query:
    */

    std::vector<int32_t>start_range(DIMENSION, 0);
    std::vector<int32_t>end_range(DIMENSION, 0);

    for (dimension_t i = 0; i < DIMENSION; i++){
        start_range[i] = min_values[i];
        end_range[i] = max_values[i];

        if (i == 0)  // Creation Time
        {
            start_range[i] = 1300000000;  
            end_range[i] = 1400000000;
        }
        if (i == 1){  // Modify Time
            start_range[i] = 1399000000;  
            end_range[i] = 1400000000;
        }
        if (i == 6)  // File Size
        {
            start_range[i] = 10000;
            end_range[i] = 2147483647;
        }
    }
    std::vector<int32_t> found_points;
    start = GetTimestamp();
    client.range_search_trie(found_points, start_range, end_range);
    diff = GetTimestamp() - start;

    std::cout << found_points.size() << std::endl;
    std::cout << "Range Search end to end latency: " << diff << std::endl;   

    delete data_vector;
    return 0; 
}