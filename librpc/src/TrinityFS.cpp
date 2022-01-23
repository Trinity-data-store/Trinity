#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "MDTrieShardClient.h"
#include "TrinityBenchShared.h"
#include "trie.h"
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
  FILE *fp = fopen("../data/fs/fs_dataset.txt", "r");
  if (fp == nullptr)
  {
      fprintf(stderr, "file not found\n");
      exit(EXIT_FAILURE);
  }

  n_leaves_t n_points = 0;
  n_leaves_t n_lines = 14583357;
  total_points_count = n_lines;
  auto data_vector = new vector<vector <int32_t>>;

  while ((read = getline(&line, &len, fp)) != -1)
  {
      vector <int32_t> point(DIMENSION, 0);
      char *token = strtok(line, " ");
      char *ptr;

      for (uint8_t i = 1; i <= 2; i ++){
          token = strtok(nullptr, " ");
      }

      for (dimension_t i = 0; i < DIMENSION; i++){
          token = strtok(nullptr, " ");
          point[i] = strtoull(token, &ptr, 10) % std::numeric_limits<int32_t>::max();
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

  return data_vector;
}


int main(){

    std::vector<std::string> server_ips = {"172.28.229.152", "172.28.229.153", "172.28.229.151", "172.28.229.149", "172.28.229.148"};
    total_points_count = 14583357;
    int shard_num = 20;
    int client_num = 1;
    auto client = MDTrieClient(server_ips, shard_num);
    if (!client.ping(0)){
        std::cerr << "Server setting wrong!" << std::endl;
        exit(-1);
    }

    TimeStamp start, diff;
    cout << "Storage: " << client.get_size() << endl;

    /** 
        Insert all points
    */

    std::tuple<uint32_t, float> return_tuple;
    uint32_t throughput;

    std::vector<int32_t> max_values(DIMENSION, 0);
    std::vector<int32_t> min_values(DIMENSION, 2147483647);
    vector<vector <int32_t>> *data_vector = get_data_vector(max_values, min_values);

    start = GetTimestamp();
    throughput = total_client_insert(data_vector, shard_num, client_num, server_ips, &client);
    diff = GetTimestamp() - start;

    cout << "Insertion Throughput (pt / seconds): " << throughput << endl;
    cout << "End-to-end Latency (us): " << diff << endl;
    cout << "Storage: " << client.get_size() << endl;

    /**   
     * Sample Query:
     * (1) Find the top 5 users who created OR modified a certain file in this time window. (sort by number of files created)
    */

    start = GetTimestamp();

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
            start_range[i] = 0;
            end_range[i] = 2147483647;
        }
    }
    std::vector<int32_t> found_points;

    client.range_search_trie(found_points, start_range, end_range);
    
    // std::map<int32_t, int32_t> user_ids_to_mod_count;

    // for (unsigned int i = 0; i < found_points.size(); i++){
    //     std::vector<int32_t> point;
    //     client.primary_key_lookup(point, found_points[i]);
    //     if (user_ids_to_mod_count.find(point[4]) == user_ids_to_mod_count.end()){  // point[4] owner_id
    //         user_ids_to_mod_count[point[4]] = 1;
    //     }
    //     else {
    //         user_ids_to_mod_count[point[4]] += 1;
    //     }
    // }

    int sent_count = 0;
    for (unsigned i = 0; i < found_points.size(); i++){

        if (sent_count != 0 && sent_count % 20 == 0){
        for (uint32_t j = i - sent_count; j < i; j++){
            std::vector<int32_t> rec_vect;
            client.primary_key_lookup_rec(rec_vect, found_points[j]);
        }
        sent_count = 0;
        }
        client.primary_key_lookup_send(found_points[i]);
        sent_count ++;
    }
    for (uint32_t j = found_points.size() - sent_count; j < found_points.size(); j++){
        std::vector<int32_t> rec_vect;
        client.primary_key_lookup_rec(rec_vect, found_points[j]);
    }

    diff = GetTimestamp() - start;
    std::cout << "Query 1 end to end latency: " << diff << std::endl;   
    std::cout << "Found: " << found_points.size() << std::endl;
    int count = 0;
    for (unsigned int i = 0; i < data_vector->size(); i++){
        std::vector<int32_t> data = (* data_vector)[i];
        if (data[0] >= 1300000000 && data[0] <= 1400000000 && data[1] >= 1399000000 && data[1] <= 1400000000 && data[6] >= 0 && data[6] <= 2147483647)
            count ++;
    }
    std::cout << "Correct Size: " << count << std::endl;

    /**   
     * (2) Find the top5 files by size created and modified within a 100 second time window.
    */

    start = GetTimestamp();

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
            start_range[i] = 0;
            end_range[i] = 2147483647;
        }
    }
    found_points.clear();

    client.range_search_trie(found_points, start_range, end_range);
    // std::vector<int32_t> found_sizes;

    // for (unsigned int i = 0; i < found_points.size(); i++){
    //     std::vector<int32_t> point;
    //     client.primary_key_lookup(point, found_points[i]);
    //     found_sizes.push_back(point[6]); // point[6] size
    // }

    sent_count = 0;
    for (unsigned i = 0; i < found_points.size(); i++){

        if (sent_count != 0 && sent_count % 20 == 0){
        for (uint32_t j = i - sent_count; j < i; j++){
            std::vector<int32_t> rec_vect;
            client.primary_key_lookup_rec(rec_vect, found_points[j]);
        }
        sent_count = 0;
        }
        client.primary_key_lookup_send(found_points[i]);
        sent_count ++;
    }
    for (uint32_t j = found_points.size() - sent_count; j < found_points.size(); j++){
        std::vector<int32_t> rec_vect;
        client.primary_key_lookup_rec(rec_vect, found_points[j]);
    }

    diff = GetTimestamp() - start;
    std::cout << "Query 2 end to end latency: " << diff << std::endl;   
    std::cout << "Found: " << found_points.size() << std::endl;

    count = 0;
    for (unsigned int i = 0; i < data_vector->size(); i++){
        std::vector<int32_t> data = (* data_vector)[i];
        if (data[0] >= 1300000000 && data[0] <= 1400000000 && data[1] >= 1399000000 && data[1] <= 1400000000 && data[6] >= 0 && data[6] <= 2147483647)
            count ++;
    }
    std::cout << "Correct Size: " << count << std::endl;

    for (unsigned int i = 0; i < found_points.size(); i++){
        std::vector<int32_t> data = (* data_vector)[found_points[i]];
        bool correct = false;
        if (data[0] >= 1300000000 && data[0] <= 1400000000 && data[1] >= 1399000000 && data[1] <= 1400000000 && data[6] >= 0 && data[6] <= 2147483647)
            correct = true;
        if (!correct)
            std::cout << "Incorrect!" << std::endl;
    }

    /**   
     * (3) Average, minimum and maximum file size across16 fixed-sized adjacent 50 second time windows.
    */

    start = GetTimestamp();

    for (int j = 1000000000; j <= 1800000000; j += 50000000){
        for (dimension_t i = 0; i < 7; i++){
            start_range[i] = min_values[i];
            end_range[i] = max_values[i];

            if (i == 1){
                start_range[i] = j;  
                end_range[i] = j + 50000000;
            }
            if (i == 0)
            {
                start_range[i] = j;  
                end_range[i] = j + 50000000;
            }
            if (i == 6)
            {
                start_range[i] = 0;
                end_range[i] = 2147483647;
            }
        }
        found_points.clear();
        client.range_search_trie(found_points, start_range, end_range);
        // std::vector<int32_t> found_sizes;

        // for (unsigned int i = 0; i < found_points.size(); i++){
        //     std::vector<int32_t> point;
        //     client.primary_key_lookup(point, found_points[i]);
        //     found_sizes.push_back(point[6]); // point[6] size
        // }

        // sent_count = 0;
        // for (unsigned i = 0; i < found_points.size(); i++){

        //     if (sent_count != 0 && sent_count % 20 == 0){
        //     for (uint32_t j = i - sent_count; j < i; j++){
        //         std::vector<int32_t> rec_vect;
        //         client.primary_key_lookup_rec(rec_vect, found_points[j]);
        //     }
        //     sent_count = 0;
        //     }
        //     client.primary_key_lookup_send(found_points[i]);
        //     sent_count ++;
        // }
        // for (uint32_t j = found_points.size() - sent_count; j < found_points.size(); j++){
        //     std::vector<int32_t> rec_vect;
        //     client.primary_key_lookup_rec(rec_vect, found_points[j]);
        // }
    }
    diff = GetTimestamp() - start;
    std::cout << "Query 3 end to end latency: " << diff << std::endl;   
    
    /**   
        Point Lookup given primary key
    */

    start = GetTimestamp();
    throughput = total_client_lookup(data_vector, shard_num, client_num, server_ips, &client);

    diff = GetTimestamp() - start;
    cout << "Primary Key Lookup Throughput (pt / seconds): " << throughput << endl;

    client.clear_trie();
    delete data_vector;
    return 0; 
}