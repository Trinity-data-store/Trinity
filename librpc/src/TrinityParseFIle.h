
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
const int DIMENSION_NYC = 15;
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
    // int real_index = -1;
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
        /*
        if (index == 8 || index == 9 || index == 10)
            continue;
        real_index ++;
        */
        point[index] = static_cast<int32_t>(std::stoul(substr));
    }
    index ++; 
    // real_index ++;
    std::string substr = line.substr(start, end - start); 
    point[index] = static_cast<int32_t>(std::stoul(substr));


    for (dimension_t i = 0; i < DIMENSION_GITHUB; i++){
        if (i == 8 || i == 9) {
            point[i] -= 20110000;
        }
    }

    return point;
}

std::vector<int32_t> parse_line_nyc(std::string line) {

    vector <int32_t> point(DIMENSION_NYC, 0);
    bool primary_key = true;
    std::string delim = ",";
    auto start = 0U;
    auto end = line.find(delim);
    int real_index = -1;
    // pickup_date, dropoff_date, pickup_longitude, pickup_latitude, dropoff_longitude, dropoff_latitude, passenger_count, trip_distance, fare_amount, extra, mta_tax, tip_amount, tolls_amount, improvement_surcharge, total_amount
    while (end != std::string::npos)
    {
        std::string substr = line.substr(start, end - start); 
        start = end + 1;
        end = line.find(delim, start);

        if (primary_key) {
            primary_key = false;
            continue;
        }

        real_index ++;

        if (real_index >= 2 && real_index <= 5){
            float num_float = std::stof(substr);
            point[real_index] = static_cast<int32_t>(num_float * 10);
        }
        // else if (real_index >= 7){
        //     float num_float = std::stof(substr);
        //     point[real_index] = static_cast<int32_t>(num_float * 10);
        // }
        else {
            point[real_index] = static_cast<int32_t>(std::stoul(substr));
        }
    }

    real_index ++;
    std::string substr = line.substr(start, end - start); 
    point[real_index] = static_cast<int32_t>(std::stoul(substr));

    point[0] -= 20090000;
    point[1] -= 19700000;


    // for (int i = 0; i < DIMENSION_NYC; i++) 
    //     std::cout << point[i] << " ";
    // std::cout << std::endl;
    // exit(0);
    return point;
}

void update_range_search_range_tpch(std::vector<int32_t> &start_range, std::vector<int32_t> &end_range, std::string line) {

    std::stringstream ss(line);

    while (ss.good()) {

        std::string index_str;
        std::getline(ss, index_str, ',');

        std::string start_range_str;
        std::getline(ss, start_range_str, ',');
        std::string end_range_str;
        std::getline(ss, end_range_str, ',');

        if (start_range_str != "-1") {
            start_range[static_cast<int32_t>(std::stoul(index_str))] = static_cast<int32_t>(std::stoul(start_range_str));
        }
        if (end_range_str != "-1") {
            end_range[static_cast<int32_t>(std::stoul(index_str))] = static_cast<int32_t>(std::stoul(end_range_str));
        }
    }
    for (dimension_t i = 0; i < start_range.size(); i++){
        if (i >= 4 && i != 7) {
            start_range[i] -= 19000000;
            end_range[i] -= 19000000;
        }
    }
}

void update_range_search_range_nyc(std::vector<int32_t> &start_range, std::vector<int32_t> &end_range, std::string line) {

    std::stringstream ss(line);

    while (ss.good()) {

        std::string index_str;
        std::getline(ss, index_str, ',');

        std::string start_range_str;
        std::getline(ss, start_range_str, ',');
        std::string end_range_str;
        std::getline(ss, end_range_str, ',');

        int index = std::stoul(index_str);

        if (start_range_str != "-1") {
            if (index >= 2 && index <= 5) {
                float num_float = std::stof(start_range_str);
                start_range[static_cast<int32_t>(index)] = static_cast<int32_t>(num_float * 10);                    
            }
            else 
                start_range[static_cast<int32_t>(index)] = static_cast<int32_t>(std::stoul(start_range_str));
        }
        if (end_range_str != "-1") {
            if (index >= 2 && index <= 5) {
                float num_float = std::stof(end_range_str);
                end_range[static_cast<int32_t>(index)] = static_cast<int32_t>(num_float * 10);                    
            }
            else 
                end_range[static_cast<int32_t>(index)] = static_cast<int32_t>(std::stoul(end_range_str));
        }
    }
    start_range[0] -= 20090000;
    start_range[1] -= 19700000;
    end_range[0] -= 20090000;
    end_range[1] -= 19700000;
}

void update_range_search_range_github(std::vector<int32_t> &start_range, std::vector<int32_t> &end_range, std::string line) {

    std::stringstream ss(line);

    while (ss.good()) {

        std::string index_str;
        std::getline(ss, index_str, ',');

        std::string start_range_str;
        std::getline(ss, start_range_str, ',');
        std::string end_range_str;
        std::getline(ss, end_range_str, ',');

        int index = std::stoul(index_str);
        if (index > 10)
            index -= 3;

        if (start_range_str != "-1") {
            start_range[static_cast<int32_t>(index)] = static_cast<int32_t>(std::stoul(start_range_str));
        }
        if (end_range_str != "-1") {
            end_range[static_cast<int32_t>(index)] = static_cast<int32_t>(std::stoul(end_range_str));
        }
    }

    for (dimension_t i = 0; i < start_range.size(); i++){
        if (i >= 8 || i == 9) {
            start_range[i] -= 20110000;
            end_range[i] -= 20110000;
        }
    }
    
}

#endif //TrinityParseFile_H
