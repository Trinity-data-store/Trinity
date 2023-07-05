#ifndef TrinityCommon_H
#define TrinityCommon_H

#include "trie.h"
#include <climits>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#define TPCH_MICRO_SIZE 250000000   // 250M
#define GITHUB_MICRO_SIZE 200000000 // 200M
#define NYC_MICRO_SIZE 200000000    // 200M

#define TPCH_SIZE 1000000000  // 1B
#define GITHUB_SIZE 828056295 // 828M
#define NYC_SIZE 675200000    // 675M

#define TPCH_DIMENSION 9
#define GITHUB_DIMENSION 10
#define NYC_DIMENSION 15

#define QUERY_NUM 1000
// #define TEST_STORAGE

enum
{
  TPCH = 1,
  GITHUB = 2,
  NYC = 3,
};

std::string TPCH_DATA_ADDR = "/mntData/tpch_dataset.csv";
std::string GITHUB_DATA_ADDR = "/mntData/github_dataset.csv";
std::string NYC_DATA_ADDR = "/mntData/nyc_dataset.csv";

std::string TPCH_SPLIT_ADDR = "/mntData/tpch_split/";
std::string GITHUB_SPLIT_ADDR = "/mntData/github_split/";
std::string NYC_SPLIT_ADDR = "/mntData/nyc_split/";

std::string TPCH_QUERY_ADDR = "/proj/trinity-PG0/Trinity/queries/tpch_query";
std::string GITHUB_QUERY_ADDR = "/proj/trinity-PG0/Trinity/queries/github_query";
std::string NYC_QUERY_ADDR = "/proj/trinity-PG0/Trinity/queries/nyc_query";

unsigned int skip_size_count = 0;
/* Because it results in otherwise OOM for other benchmarks. */
int micro_tpch_size = 250000000;
int micro_github_size = 200000000;
int micro_nyc_size = 200000000;
bool is_microbenchmark = false;

enum
{
  OPTIMIZATION_SM = 0, /* Default*/
  OPTIMIZATION_B = 1,
  OPTIMIZATION_CN = 2,
  OPTIMIZATION_GM = 3,
};

level_t max_depth = 32;
level_t trie_depth = 6;
preorder_t max_tree_node = 512;
point_t points_to_insert = 1000;
point_t points_to_lookup = 1000;
std::string results_folder_addr = "/proj/trinity-PG0/Trinity/results/";
std::string identification_string = "";
int optimization_code = OPTIMIZATION_SM;
std::string optimization = "SM";
float selectivity_upper = 0.0015;
float selectivity_lower = 0.0005;

/* [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE,
 * TOTALPRICE, ORDERDATE] */
std::vector<int32_t> tpch_max_values = { 50,       10494950, 10,
                                         8,        19981201, 19981031,
                                         19981231, 58063825, 19980802 };
std::vector<int32_t> tpch_min_values = { 1,        90000,    0,
                                         0,        19920102, 19920131,
                                         19920103, 81300,    19920101 };

/* [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE,
 * TOTALPRICE, ORDERDATE] */
std::vector<int32_t> github_max_values = { 7451541,  737170,  262926, 354850,
                                           379379,   3097263, 703341, 8745,
                                           20201206, 20201206 };
std::vector<int32_t> github_min_values = { 1, 1, 0, 0,        0,
                                           0, 0, 0, 20110211, 20110211 };

std::vector<int32_t> nyc_max_values = { 20160630, 20221220, 899,     898,
                                        899,      898,      255,     198623000,
                                        21474808, 1000,     1312,    3950589,
                                        21474836, 138,      21474830 };
std::vector<int32_t> nyc_min_values = { 20090101, 19700101, 0, 0, 0, 0, 0, 0,
                                        0,        0,        0, 0, 0, 0, 0 };

int
gen_rand(int start, int end)
{
  return start + (std::rand() % (end - start + 1));
}

void
use_nyc_setting(int dimensions, int _total_points_count)
{

  std::vector<level_t> bit_widths = { 18,      20, 10, 10,     10 + 18,
                                      10 + 18, 8,  28, 25,     10 + 18,
                                      11 + 17, 22, 25, 8 + 20, 25 };
  std::vector<level_t> start_bits = { 0,      0, 0, 0,      0 + 18,
                                      0 + 18, 0, 0, 0,      0 + 18,
                                      0 + 17, 0, 0, 0 + 20, 0 };
  total_points_count = _total_points_count;
  is_collapsed_node_exp = false;

  if (optimization_code == OPTIMIZATION_B) {
    bit_widths = { 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28 };
    start_bits = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    identification_string = "_B";
    total_points_count = micro_nyc_size / 10;
    is_collapsed_node_exp = true;
  }

  if (optimization_code == OPTIMIZATION_CN) {
    bit_widths = { 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28 };
    start_bits = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    total_points_count = micro_nyc_size;
    identification_string = "_CN";
  }

  if (optimization_code == OPTIMIZATION_GM) {
    bit_widths = {
      18, 20, 10, 10, 10, 10, 8, 28, 25, 10, 11, 22, 25, 8, 25
    }; // 15 Dimensions;
    start_bits = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }; // 15 Dimensions;
    total_points_count = micro_nyc_size;
    identification_string = "_GM";
  }

  start_bits.resize(dimensions);
  bit_widths.resize(dimensions);

  trie_depth = 6;
  max_depth = 28;
  no_dynamic_sizing = true;

  create_level_to_num_children(bit_widths, start_bits, max_depth);
}

void
use_github_setting(int dimensions, int _total_points_count)
{

  std::vector<level_t> bit_widths = {
    24, 24, 24, 24, 24, 24, 24, 16, 24, 24
  }; // 10 Dimensions;
  std::vector<level_t> start_bits = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  }; // 10 Dimensions;
  total_points_count = _total_points_count;
  is_collapsed_node_exp = false;
  if (is_microbenchmark)
    skip_size_count = 500000000;

  if (optimization_code == OPTIMIZATION_B) {
    bit_widths = { 24, 24, 24, 24, 24, 24, 24, 24, 24, 24 };
    start_bits = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    identification_string = "_B";
    total_points_count =
      micro_github_size / 5; /* Otherwise quiery will return too few points */
    is_collapsed_node_exp = true;
  }

  if (optimization_code == OPTIMIZATION_CN) {
    bit_widths = { 24, 24, 24, 24, 24, 24, 24, 24, 24, 24 };
    start_bits = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    identification_string = "_CN";
    total_points_count = micro_github_size;
  }

  if (optimization_code == OPTIMIZATION_GM) {
    bit_widths = { 24, 24, 24, 24, 24, 24, 24, 16, 24, 24 };
    start_bits = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    identification_string = "_GM";
    total_points_count = micro_github_size;
  }

  start_bits.resize(dimensions);
  bit_widths.resize(dimensions);

  trie_depth = 6;
  max_depth = 24;
  no_dynamic_sizing = true;
  max_tree_node = 512;

  create_level_to_num_children(bit_widths, start_bits, max_depth);
}

void
use_tpch_setting(int dimensions, int _total_points_count)
{ /* An extra dimensions input for sensitivity experiment. */

  std::vector<level_t> bit_widths = {
    8, 32, 16, 24, 20, 20, 20, 32, 20
  }; // 9 Dimensions;
  std::vector<level_t> start_bits = {
    0, 0, 8, 16, 0, 0, 0, 0, 0
  }; // 9 Dimensions;
  total_points_count = _total_points_count;
  is_collapsed_node_exp = false;

  if (optimization_code == OPTIMIZATION_B) {
    bit_widths = { 32, 32, 32, 32, 32, 32, 32, 32, 32 };
    start_bits = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    identification_string = "_B";
    total_points_count = micro_tpch_size / 10;
    is_collapsed_node_exp = true;
  }

  if (optimization_code == OPTIMIZATION_CN) {
    bit_widths = { 32, 32, 32, 32, 32, 32, 32, 32, 32 };
    start_bits = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    identification_string = "_CN";
    total_points_count = micro_tpch_size / 5; /* Otherwise Will be too slow */
  }

  if (optimization_code == OPTIMIZATION_GM) {
    bit_widths = { 8, 32, 16, 24, 20, 20, 20, 32, 20 };
    start_bits = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    identification_string = "_GM";
    total_points_count = micro_tpch_size;
  }

  start_bits.resize(dimensions);
  bit_widths.resize(dimensions);

  trie_depth = 6;
  max_depth = 32;
  no_dynamic_sizing = true;
  create_level_to_num_children(bit_widths, start_bits, max_depth);
}

void
flush_vector_to_file(std::vector<TimeStamp> vect, std::string filename)
{

  std::ofstream outFile(filename);
  for (const auto& e : vect)
    outFile << std::to_string(e) << "\n";
}

void
flush_string_to_file(std::string str, std::string filename)
{

  std::ofstream outFile(filename);
  outFile << str << "\n";
}

template<dimension_t DIMENSION>
void
get_query_nyc(std::string line,
              data_point<DIMENSION>* start_range,
              data_point<DIMENSION>* end_range)
{

  for (dimension_t i = 0; i < DIMENSION; i++) {
    start_range->set_coordinate(i, nyc_min_values[i]);
    end_range->set_coordinate(i, nyc_max_values[i]);
  }
  std::stringstream ss(line);
  while (ss.good()) {

    std::string index_str;
    std::getline(ss, index_str, ',');

    std::string start_range_str;
    std::getline(ss, start_range_str, ',');
    std::string end_range_str;
    std::getline(ss, end_range_str, ',');

    dimension_t index = std::stoul(index_str);
    if (start_range_str != "-1" && index < DIMENSION) {
      if (index >= 2 && index <= 5) {
        float num_float = std::stof(start_range_str);
        start_range->set_coordinate(index,
                                    static_cast<int32_t>(num_float * 10));
      } else
        start_range->set_coordinate(index, std::stoul(start_range_str));
    }
    if (end_range_str != "-1" && index < DIMENSION) {
      if (index >= 2 && index <= 5) {
        float num_float = std::stof(end_range_str);
        end_range->set_coordinate(index, static_cast<int32_t>(num_float * 10));
      } else
        end_range->set_coordinate(index, std::stoul(end_range_str));
    }
  }

  start_range->set_coordinate(0, start_range->get_coordinate(0) - 20090000);
  start_range->set_coordinate(1, start_range->get_coordinate(1) - 19700000);
  end_range->set_coordinate(0, end_range->get_coordinate(0) - 20090000);
  end_range->set_coordinate(1, end_range->get_coordinate(1) - 19700000);
}

template<dimension_t DIMENSION>
void
get_query_github(std::string line,
                 data_point<DIMENSION>* start_range,
                 data_point<DIMENSION>* end_range)
{

  for (dimension_t i = 0; i < DIMENSION; i++) {
    start_range->set_coordinate(i, github_min_values[i]);
    end_range->set_coordinate(i, github_max_values[i]);
  }
  std::stringstream ss(line);
  while (ss.good()) {

    std::string index_str;
    std::getline(ss, index_str, ',');

    std::string start_range_str;
    std::getline(ss, start_range_str, ',');
    std::string end_range_str;
    std::getline(ss, end_range_str, ',');

    dimension_t index = std::stoul(index_str);
    if (index > 10)
      index -= 3;

    if (start_range_str != "-1" && index < DIMENSION) {
      start_range->set_coordinate(index, std::stoul(start_range_str));
    }
    if (end_range_str != "-1" && index < DIMENSION) {
      end_range->set_coordinate(index, std::stoul(end_range_str));
    }
  }

  for (dimension_t i = 0; i < GITHUB_DIMENSION; i++) {
    if (i >= 8) {
      start_range->set_coordinate(i, start_range->get_coordinate(i) - 20110000);
      end_range->set_coordinate(i, end_range->get_coordinate(i) - 20110000);
    }
  }
}

template<dimension_t DIMENSION>
void
get_query_tpch(std::string line,
               data_point<DIMENSION>* start_range,
               data_point<DIMENSION>* end_range)
{

  for (dimension_t i = 0; i < DIMENSION; i++) {
    start_range->set_coordinate(i, tpch_min_values[i]);
    end_range->set_coordinate(i, tpch_max_values[i]);
  }

  std::stringstream ss(line);

  while (ss.good()) {

    std::string index_str;
    std::getline(ss, index_str, ',');

    std::string start_range_str;
    std::getline(ss, start_range_str, ',');
    std::string end_range_str;
    std::getline(ss, end_range_str, ',');

    if (start_range_str != "-1" && std::stoul(index_str) < DIMENSION) {
      start_range->set_coordinate(std::stoul(index_str),
                                  std::stoul(start_range_str));
    }
    if (end_range_str != "-1" && std::stoul(index_str) < DIMENSION) {
      end_range->set_coordinate(std::stoul(index_str),
                                std::stoul(end_range_str));
    }
  }

  for (dimension_t i = 0; i < DIMENSION; i++) {
    if (i >= 4 && i != 7) {
      start_range->set_coordinate(i, start_range->get_coordinate(i) - 19000000);
      end_range->set_coordinate(i, end_range->get_coordinate(i) - 19000000);
    }
  }
}

template<dimension_t DIMENSION>
void
get_random_query_tpch(data_point<DIMENSION>* start_range,
                      data_point<DIMENSION>* end_range)
{

  for (dimension_t i = 0; i < DIMENSION; i++) {
    start_range->set_coordinate(
      i, gen_rand(tpch_min_values[i], tpch_max_values[i]));
    end_range->set_coordinate(
      i, gen_rand(start_range->get_coordinate(i), tpch_max_values[i]));
  }

  for (dimension_t i = 0; i < DIMENSION; i++) {
    if (i >= 4 && i != 7) {
      start_range->set_coordinate(i, start_range->get_coordinate(i) - 19000000);
      end_range->set_coordinate(i, end_range->get_coordinate(i) - 19000000);
    }
  }
}

#endif // TrinityCommon_H
