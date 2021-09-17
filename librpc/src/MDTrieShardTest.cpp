
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

  FILE *fp = fopen("../libmdtrie/bench/data/osm_combined_updated.csv", "r");
  char *line = nullptr;
  size_t len = 0;
  ssize_t read;
  // If the file cannot be open
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

int main(int argc, char *argv[]){
  
  if (argc != 3){
    cerr << "wrong number of arguments" << endl;
  }

  auto client = MDTrieClient(9090, 10);
  // auto client = MDTrieClient(atoi(argv[1]), atoi(argv[2]));

  client.ping();

  vector<vector <int32_t>> *data_vector = get_data_vector();

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
}