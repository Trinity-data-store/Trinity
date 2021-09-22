
#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "MDTrieShardClient.h"
#include "trie.h"
#include <tqdm.h>
#include <future>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

const int DIMENSION = 6; 
n_leaves_t n_lines = 14252681;

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

int main(){
  
  auto client = MDTrieClient(9090, 10);

  client.ping();

  vector<vector <int32_t>> *data_vector = get_data_vector();

/** 
    Insert all points from the OSM dataset
*/

  tqdm bar;
  TimeStamp diff = 0;
  TimeStamp start;

  for (uint32_t i = 0; i < n_lines; i ++){
    bar.progress(i, n_lines);
    vector <int32_t> point = (* data_vector)[i];

    start = GetTimestamp();
    client.insert(point, i);
    diff += GetTimestamp() - start;
  }

  cout << "Insertion latency per point: " << (float) diff / n_lines << " us/point" << endl;
  bar.finish();
  client.get_time();

// /** 
//     Range Search full range
// */

  auto start_range = vector <int32_t>(DIMENSION, 0);
  auto end_range = vector <int32_t>(DIMENSION, 1 << 31);
  
  std::vector<int32_t> return_vect;

  diff = 0;
  start = GetTimestamp();
  client.range_search_trie(return_vect, start_range, end_range);
  diff = GetTimestamp() - start;

  cout << "number of points found: " << return_vect.size() << endl;
  cout << "Range Search Latency per found points: " << (float) diff / return_vect.size() << " us/point" << endl;
  cout << "outer vector time: " << (float) thrift_vector_time / return_vect.size() << endl;
  client.get_time();

// /** 
//     Check if all inserted points are present
// */

  tqdm bar1;
  diff = 0;
  for (uint32_t i = 0; i < n_lines; i ++){
    bar1.progress(i, n_lines);
    vector <int32_t> point = (* data_vector)[i];
    start = GetTimestamp();
    if (!client.check(point, i)){
      cout << "point not found!" << endl;
    }
    diff += GetTimestamp() - start;
  }
  bar1.finish();
  cout << "Point Query latency per point: " << (float) diff / n_lines << " us/point" << endl;
  client.get_time();

/** 
    Lookup Primary
*/

  tqdm bar2;
  diff = 0;
  for (uint32_t i = 0; i < n_lines; i ++){
    bar2.progress(i, n_lines);
    vector <int32_t> point = (* data_vector)[i];
    vector <int32_t> found_point; 

    start = GetTimestamp();
    client.primary_key_lookup(found_point, i);

    diff += GetTimestamp() - start;

    if (point != found_point){
      cout << "Lookup found vector not equal!" << endl;
      raise(SIGINT);
      return -1;
    }
  }
  
  bar2.finish();
  cout << "Lookup latency per point: " << (float) diff / n_lines << " us/point" << endl;
  client.get_time();
  return 0;
}