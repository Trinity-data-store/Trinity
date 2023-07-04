
#ifndef TrinityParseFile_H
#define TrinityParseFile_H

#include "common.hpp"
#include <atomic>
#include <fstream>
#include <future>
#include <iostream>
#include <tuple>
#include <vector>

using namespace std;

// Parse one line from TPC-H file.
std::vector<int32_t>
parse_line_tpch(std::string line)
{

  vector<int32_t> point(TPCH_DIMENSION, 0);
  int index = -1;
  bool primary_key = true;
  std::string delim = ",";
  auto start = 0U;
  auto end = line.find(delim);

  // [id, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE,
  // RECEIPTDATE, TOTALPRICE, ORDERDATE]
  while (end != std::string::npos) {
    std::string substr = line.substr(start, end - start);
    start = end + 1;
    end = line.find(delim, start);

    if (primary_key) {
      primary_key = false;
      continue;
    }
    index++;
    point[index] = static_cast<int32_t>(std::stoul(substr));
  }
  index++;
  std::string substr = line.substr(start, end - start);
  point[index] = static_cast<int32_t>(std::stoul(substr));

  for (int i = 0; i < TPCH_DIMENSION; i++) {
    if (i >= 4 && i != 7) {
      point[i] -= 19000000;
    }
  }

  return point;
}

// Parse one line from TPC-H file.
std::vector<int32_t>
parse_line_github(std::string line)
{

  vector<int32_t> point(GITHUB_DIMENSION, 0);
  int index = -1;
  bool primary_key = true;
  std::string delim = ",";
  auto start = 0U;
  auto end = line.find(delim);
  // int real_index = -1;
  // [id, events_count, authors_count, forks, stars, issues, pushes, pulls,
  // downloads, adds, dels, add_del_ratio, start_date, end_date]
  while (end != std::string::npos) {
    std::string substr = line.substr(start, end - start);
    start = end + 1;
    end = line.find(delim, start);

    if (primary_key) {
      primary_key = false;
      continue;
    }
    index++;
    point[index] = static_cast<int32_t>(std::stoul(substr));
  }
  index++;
  std::string substr = line.substr(start, end - start);
  point[index] = static_cast<int32_t>(std::stoul(substr));

  for (int i = 0; i < GITHUB_DIMENSION; i++) {
    if (i == 8 || i == 9) {
      point[i] -= 20110000;
    }
  }

  return point;
}

std::vector<int32_t>
parse_line_nyc(std::string line)
{

  vector<int32_t> point(NYC_DIMENSION, 0);
  bool primary_key = true;
  std::string delim = ",";
  auto start = 0U;
  auto end = line.find(delim);
  int real_index = -1;

  // pickup_date, dropoff_date, pickup_longitude, pickup_latitude,
  // dropoff_longitude, dropoff_latitude, passenger_count, trip_distance,
  // fare_amount, extra, mta_tax, tip_amount, tolls_amount,
  // improvement_surcharge, total_amount
  while (end != std::string::npos) {
    std::string substr = line.substr(start, end - start);
    start = end + 1;
    end = line.find(delim, start);

    if (primary_key) {
      primary_key = false;
      continue;
    }

    real_index++;

    if (real_index >= 2 && real_index <= 5) {
      float num_float = std::stof(substr);
      point[real_index] = static_cast<int32_t>(num_float * 10);
    } else {
      point[real_index] = static_cast<int32_t>(std::stoul(substr));
    }
  }

  real_index++;
  std::string substr = line.substr(start, end - start);
  point[real_index] = static_cast<int32_t>(std::stoul(substr));

  point[0] -= 20090000;
  point[1] -= 19700000;

  return point;
}

void
update_range_search_range_tpch(std::vector<int32_t>& start_range,
                               std::vector<int32_t>& end_range,
                               std::string line)
{

  std::stringstream ss(line);

  while (ss.good()) {

    std::string index_str;
    std::getline(ss, index_str, ',');

    std::string start_range_str;
    std::getline(ss, start_range_str, ',');
    std::string end_range_str;
    std::getline(ss, end_range_str, ',');

    if (start_range_str != "-1") {
      start_range[static_cast<int32_t>(std::stoul(index_str))] =
        static_cast<int32_t>(std::stoul(start_range_str));
    }
    if (end_range_str != "-1") {
      end_range[static_cast<int32_t>(std::stoul(index_str))] =
        static_cast<int32_t>(std::stoul(end_range_str));
    }
  }
  for (unsigned int i = 0; i < start_range.size(); i++) {
    if (i >= 4 && i != 7) {
      start_range[i] -= 19000000;
      end_range[i] -= 19000000;
    }
  }
}

void
update_range_search_range_nyc(std::vector<int32_t>& start_range,
                              std::vector<int32_t>& end_range,
                              std::string line)
{

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
        start_range[static_cast<int32_t>(index)] =
          static_cast<int32_t>(num_float * 10);
      } else
        start_range[static_cast<int32_t>(index)] =
          static_cast<int32_t>(std::stoul(start_range_str));
    }
    if (end_range_str != "-1") {
      if (index >= 2 && index <= 5) {
        float num_float = std::stof(end_range_str);
        end_range[static_cast<int32_t>(index)] =
          static_cast<int32_t>(num_float * 10);
      } else
        end_range[static_cast<int32_t>(index)] =
          static_cast<int32_t>(std::stoul(end_range_str));
    }
  }
  start_range[0] -= 20090000;
  start_range[1] -= 19700000;
  end_range[0] -= 20090000;
  end_range[1] -= 19700000;
}

void
update_range_search_range_github(std::vector<int32_t>& start_range,
                                 std::vector<int32_t>& end_range,
                                 std::string line)
{

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
      start_range[static_cast<int32_t>(index)] =
        static_cast<int32_t>(std::stoul(start_range_str));
    }
    if (end_range_str != "-1") {
      end_range[static_cast<int32_t>(index)] =
        static_cast<int32_t>(std::stoul(end_range_str));
    }
  }

  for (unsigned int i = 0; i < start_range.size(); i++) {
    if (i >= 8 || i == 9) {
      start_range[i] -= 20110000;
      end_range[i] -= 20110000;
    }
  }
}

#endif // TrinityParseFile_H
