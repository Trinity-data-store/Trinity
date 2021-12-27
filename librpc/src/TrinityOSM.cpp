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
const int DIMENSION = 9;

vector<vector <int32_t>> *get_data_vector(std::vector<int32_t> &max_values, std::vector<int32_t> &min_values){

/** 
    Get data from the OSM dataset stored in a vector
*/

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
      vector <int32_t> point(DIMENSION, 0);
      bar.progress(n_points, n_lines);
      char *token = strtok(line, ",");
      char *ptr;

      for (dimension_t i = 0; i < DIMENSION; i++){
          token = strtok(nullptr, ",");
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
    client.ping();

    TimeStamp start, diff;

    /** 
        Insert all points
    */

    std::tuple<uint32_t, float> return_tuple;
    uint32_t throughput;
    float latency;

    std::vector<int32_t> max_values(DIMENSION, 0);
    std::vector<int32_t> min_values(DIMENSION, 2147483647);
    vector<vector <int32_t>> *data_vector = get_data_vector(max_values, min_values);

    start = GetTimestamp();
    return_tuple = total_client_insert(data_vector, client_num, server_ips);
    throughput = std::get<0>(return_tuple);
    latency = std::get<1>(return_tuple);
    diff = GetTimestamp() - start;

    cout << "Insertion Throughput measured from thread (pt / seconds): " << throughput << endl;
    cout << "Throughput measured from end-to-end Laatency: " << ((float) total_points_count / diff) * 1000000 << endl;
    cout << "Latency (us): " << latency << endl;

    /**   
        Point Lookup given primary key
    */

    start = GetTimestamp();
    return_tuple = total_client_lookup(data_vector, client_num, server_ips);
    throughput = std::get<0>(return_tuple);
    latency = std::get<1>(return_tuple);

    diff = GetTimestamp() - start;
    cout << "Primary Key Lookup Throughput measured from thread (pt / seconds): " << throughput << endl;
    cout << "Latency (us): " << latency << endl;
    cout << "Throughput measured from end-to-end Laatency: " << ((float) total_points_count / diff) * 1000000 << endl;

    return 0;

}