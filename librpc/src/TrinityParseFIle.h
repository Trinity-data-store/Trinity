
#ifndef TrinityParseFile_H
#define TrinityParseFile_H

#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

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

const int DIMENSION_TPCH = 9;
const int DIMENSION_GITHUB = 10;
// Parse one line from TPC-H file.
std::vector<int32_t> parse_line_tpch(std::string line) {

    vector <int32_t> point(DIMENSION_TPCH, 0);
    int index = -1;
    bool primary_key = true;
    std::string delim = ",";
    auto start = 0U;
    auto end = line.find(delim);

    // [id, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
    while (end != std::string::npos)
    {
        std::string substr = line.substr(start, end - start); 
        start = end + 1;
        end = line.find(delim, start);

        if (primary_key) {
            primary_key = false;
            continue;
        }
        index ++;
        point[index] = static_cast<int32_t>(std::stoul(substr));
    }
    index ++; 
    std::string substr = line.substr(start, end - start); 
    point[index] = static_cast<int32_t>(std::stoul(substr));


    for (dimension_t i = 0; i < DIMENSION_TPCH; i++){
        if (i >= 4 && i != 7) {
            point[i] -= 19000000;
        }
    }

    return point;
}

vector<vector <int32_t>> *get_data_vector_tpch(std::vector<int32_t> &max_values, std::vector<int32_t> &min_values){

/** 
    Get data from the TPC-H dataset stored in a vector
*/

  std::ifstream infile("/mntData2/tpch-dbgen/data_200/orders_lineitem_merged_by_chunk.csv");
  std::string line;
  std::getline(infile, line);

  n_leaves_t n_points = 0;
  auto data_vector = new vector<vector <int32_t>>;
  bool primary_key = true;

  while (std::getline(infile, line))
  {

      n_points ++;

      if (n_points % (total_points_count / 50) == 0) 
        cout << n_points << endl;
    
      if (n_points <= 1056016192)
        continue;

      std::stringstream ss(line);
      vector <int32_t> point(DIMENSION_TPCH, 0);

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
          if (primary_key) {
              primary_key = false;
              continue;
          }
      
          int32_t num;
          // [EXTENDEDPRICE, DISCOUNT, TAX, TOTALPRICE] // float with 2dp
          if (index == 1 || index == 2 || index == 3 || index == 7) 
          {
              num = static_cast<int32_t>(std::stof(substr) * 100);
          }
          // [SHIPDATE, COMMITDATE, RECEIPTDATE, ORDERDATE]
          else if (index == 4 || index == 5 || index == 6 || index == 8) //yy-mm-dd
          {
              num = static_cast<int32_t>(std::stoul(substr));
          }
          // [QUANTITY]
          else
              num = static_cast<int32_t>(std::stoul(substr));

          point[leaf_point_index] = num;
          leaf_point_index++;
      }
 

      data_vector->push_back(point);
      for (dimension_t i = 0; i < DIMENSION_TPCH; i++){
          if (point[i] > max_values[i])
              max_values[i] = point[i];
          if (point[i] < min_values[i])
              min_values[i] = point[i];         
      }    


      if (n_points == total_points_count)
          break;
  }
  return data_vector;
}

vector<vector <int32_t>> *check_data_vector_tpch(){

/** 
    Get data from the TPC-H dataset stored in a vector
*/

  std::ifstream infile("/mntData2/tpch-dbgen/data_200/orders_lineitem_merged_by_chunk.csv");
  std::string line;
  std::getline(infile, line);

  n_leaves_t n_points = 0;
  auto data_vector = new vector<vector <int32_t>>;
  int count = 0;

  while (std::getline(infile, line))
  {

        
      std::stringstream ss(line);
      vector <int32_t> point(DIMENSION_TPCH, 0);

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
          // [EXTENDEDPRICE, DISCOUNT, TAX, TOTALPRICE] // float with 2dp
          if (index == 1 || index == 2 || index == 3 || index == 7) 
          {
              num = static_cast<int32_t>(std::stof(substr) * 100);
          }
          // [SHIPDATE, COMMITDATE, RECEIPTDATE, ORDERDATE]
          else if (index == 4 || index == 5 || index == 6 || index == 8) //yy-mm-dd
          {
              num = static_cast<int32_t>(std::stoul(substr));
          }
          // [QUANTITY]
          else
              num = static_cast<int32_t>(std::stoul(substr));

          point[leaf_point_index] = num;
          leaf_point_index++;
      }

      if (point[0] <= 24 && point[2] <= 7 && point[2] >= 5 && point[4] >= 19940101 && point[4] <= 19950101)
          count ++;
            
      
      if (n_points == total_points_count)
          break;

      n_points ++;

      if (n_points % (total_points_count / 50) == 0) 
        cout << n_points << " " << count << endl;

      if (point[0] >= 256 || point[2] >= 256 || point[3] >= 256) {
        cout << "wrong overflow" << n_points << endl;
        exit(0);
      }
  }
  std::cout << "Correct Size: " << count << std::endl;

  return data_vector;
}

// Parse one line from TPC-H file.
std::vector<int32_t> parse_line_github(std::string line) {

    vector <int32_t> point(DIMENSION_GITHUB, 0);
    int index = -1;
    bool primary_key = true;
    std::string delim = ",";
    auto start = 0U;
    auto end = line.find(delim);
    int real_index = -1;
    // [id, events_count, authors_count, forks, stars, issues, pushes, pulls, downloads, adds, dels, add_del_ratio, start_date, end_date]
    while (end != std::string::npos)
    {
        std::string substr = line.substr(start, end - start); 
        start = end + 1;
        end = line.find(delim, start);

        if (primary_key) {
            primary_key = false;
            continue;
        }
        index ++;
        if (index == 8 || index == 9 || index == 10)
            continue;
        real_index ++;
        point[real_index] = static_cast<int32_t>(std::stoul(substr));
    }
    index ++; 
    real_index ++;
    std::string substr = line.substr(start, end - start); 
    point[real_index] = static_cast<int32_t>(std::stoul(substr));


    for (dimension_t i = 0; i < DIMENSION_GITHUB; i++){
        if (i == 8 || i == 9) {
            point[i] -= 20110000;
        }
    }

    return point;
}


#endif //TrinityParseFile_H
