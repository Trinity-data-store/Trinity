
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
const int BATCH_SIZE = 1024;

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


void insert_each_client(vector<vector <int32_t>> *data_vector, int client_number, int client_index){

  auto client = MDTrieClient();
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
    sent_count ++;
  }

  for (uint32_t j = end_pos - sent_count + 1; j <= end_pos; j++){
      client.insert_rec(j);
  }

}

void total_client_insert(vector<vector <int32_t>> *data_vector, int client_number){

  std::thread *threads = new std::thread[client_number];

  for (uint8_t i = 0; i < client_number; i++){
    threads[i] = std::thread(insert_each_client, data_vector, client_number, i);
  }  
  for (uint8_t i = 0; i < client_number; i++){
    threads[i].join();
  }  
}

int main(int argc, char *argv[]){
  
  if (argc != 2){
    cout << "wrong number of arguments" << endl;
    return 0;
  }

  int client_number = atoi(argv[1]);

  auto client = MDTrieClient();

  vector<vector <int32_t>> *data_vector = get_data_vector();
  uint32_t data_vector_size = data_vector->size();
 
/** 
    Insert all points from the OSM dataset
*/

  TimeStamp diff = 0;
  TimeStamp start = GetTimestamp();
  total_client_insert(data_vector, client_number);
  diff = GetTimestamp() - start;

  cout << "Insertion Throughput (pt / seconds): " << ((float) data_vector_size / diff) * 100000 << endl;
  cout << "Inserted Points: " << client.get_count() << endl << endl;

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
  cout << "total number of data points: " << data_vector->size() << endl;
  cout << "Range Search Throughput (pt / seconds): " << ((float) return_vect.size() / diff) * 100000 << endl;

  return 0;
}