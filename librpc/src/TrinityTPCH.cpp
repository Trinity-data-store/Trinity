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
const int DIMENSION = 9;

vector<vector <int32_t>> *get_data_vector(std::vector<int32_t> &max_values, std::vector<int32_t> &min_values){

/** 
    Get data from the TPC-H dataset stored in a vector
*/

  std::ifstream infile("../data/tpc-h/tpch_dataset.csv");

  std::string line;
  std::getline(infile, line);

  n_leaves_t n_points = 0;
  n_leaves_t n_lines = 110418170;
  auto data_vector = new vector<vector <int32_t>>;

  while (std::getline(infile, line))
  {
      std::stringstream ss(line);
      vector <int32_t> point(DIMENSION, 0);

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

      for (dimension_t i = 0; i < DIMENSION; i++){
          if (point[i] > max_values[i])
              max_values[i] = point[i];
          if (point[i] < min_values[i])
              min_values[i] = point[i];         
      }    
      n_points ++;
  }
  return data_vector;
}



int main(){

    std::vector<std::string> server_ips = {"172.28.229.152", "172.28.229.153", "172.28.229.151", "172.28.229.149", "172.28.229.148"};
    total_points_count = 300005812;

    int shard_num = 20;
    int client_num = 48;
    auto client = MDTrieClient(server_ips, shard_num);
    if (!client.ping(2)){
        std::cerr << "Server setting wrong!" << std::endl;
        exit(-1);
    }
    cout << "Storage: " << client.get_size() << endl;

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
    throughput = total_client_insert(data_vector, shard_num, client_num, server_ips);
    diff = GetTimestamp() - start;

    cout << "Insertion Throughput (pt / seconds): " << throughput << endl;
    cout << "End-to-end Latency (us): " << diff << endl;
    cout << "Storage: " << client.get_size() << endl;

    /**   
     * Sample Query 1:
    */

    std::vector<int32_t>start_range(DIMENSION, 0);
    std::vector<int32_t>end_range(DIMENSION, 0);

    start = GetTimestamp();

    // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]

    for (dimension_t i = 0; i < DIMENSION; i++){
        start_range[i] = min_values[i];
        end_range[i] = max_values[i];

        if (i == 0) 
        {
            start_range[i] = 20;  
        }
        if (i == 7){ 
            start_range[i] = 1000000;  
            end_range[i] = 5000000;
        }
        if (i == 3)
            start_range[i] = 1;
    }
    std::vector<int32_t> found_points;
    start = GetTimestamp();
    client.range_search_trie(found_points, start_range, end_range);
    diff = GetTimestamp() - start;

    std::cout << "Query 1 end to end latency: " << diff << std::endl;       
    std::cout << "Found points count: " << found_points.size() << std::endl;

    int count = 0;
    for (unsigned int i = 0; i < data_vector->size(); i++){
        std::vector<int32_t> data = (* data_vector)[i];
        if (data[0] >= 20 && data[7] <= 5000000 && data[7] >= 1000000 && data[3] >= 1)
            count ++;
    }
    std::cout << "Correct Size: " << count << std::endl;
    
    /**   
     * Sample Query 2:
    */

    for (dimension_t i = 0; i < DIMENSION; i++){
        start_range[i] = min_values[i];
        end_range[i] = max_values[i];

        if (i == 1) 
        {
            end_range[i] = 10000000;  
        }
        if (i == 7){ 
            start_range[i] = 5000000;
        }
        if (i == 3)
            start_range[i] = 5;
    }
    found_points.clear();
    start = GetTimestamp();
    client.range_search_trie(found_points, start_range, end_range);
    diff = GetTimestamp() - start;

    std::cout << "Query 2 end to end latency: " << diff << std::endl;  
    std::cout << "Found points count: " << found_points.size() << std::endl;
    
    count = 0;
    for (unsigned int i = 0; i < data_vector->size(); i++){
        std::vector<int32_t> data = (* data_vector)[i];
        if (data[1] <= 10000000 && data[7] >= 5000000 && data[3] >= 5)
            count ++;
    }
    std::cout << "Correct Size: " << count << std::endl;
    
    /**   
     * Sample Query 3:
    */

    for (dimension_t i = 0; i < DIMENSION; i++){
        start_range[i] = min_values[i];
        end_range[i] = max_values[i];

        if (i == 1) 
        {
            end_range[i] = 5000000;  
        }
        if (i == 7){ 
            start_range[i] = 40000000;
        }
        if (i == 3)
            start_range[i] = 5;
    }

    found_points.clear();
    client.range_search_trie(found_points, start_range, end_range);
    diff = GetTimestamp() - start;

    std::cout << "Query 3 end to end latency: " << diff << std::endl;  
    std::cout << "Found points count: " << found_points.size() << std::endl;
    count = 0;
    for (unsigned int i = 0; i < data_vector->size(); i++){
        std::vector<int32_t> data = (* data_vector)[i];
        if (data[1] <= 5000000 && data[7] >= 40000000 && data[3] >= 5)
            count ++;
    }
    std::cout << "Correct Size: " << count << std::endl;

    return 0;

    /**   
        Point Lookup given primary key
    */

    start = GetTimestamp();
    throughput = total_client_lookup(data_vector, shard_num, client_num, server_ips);

    diff = GetTimestamp() - start;
    cout << "Primary Key Lookup Throughput (pt / seconds): " << throughput << endl;


    client.clear_trie();
    delete data_vector;
    return 0;
}