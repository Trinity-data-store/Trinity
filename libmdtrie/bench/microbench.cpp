#include "benchmark.hpp"
#include "common.hpp"
#include "parser.hpp"
#include "trie.h"
#include <climits>
#include <fstream>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

void github_bench(void)
{
  std::vector<level_t> start_bits;
  std::vector<level_t> bit_widths;
  use_github_setting(GITHUB_DIMENSION, micro_github_size, bit_widths, start_bits);
  md_trie<GITHUB_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node, bit_widths, start_bits);
  MdTrieBench<GITHUB_DIMENSION> bench(&mdtrie);
  std::string folder_name = "microbenchmark/";
  if (identification_string != "")
  {
    folder_name = "optimization/";
  }
  bench.insert(GITHUB_DATA_ADDR,
               folder_name + "github_insert" + identification_string,
               total_points_count,
               parse_line_github);
  bench.get_storage(folder_name + "github_storage" + identification_string);
  bench.lookup(folder_name + "github_lookup" + identification_string);
  bench.range_search(GITHUB_QUERY_ADDR,
                     folder_name + "github_query" + identification_string,
                     get_query_github<GITHUB_DIMENSION>);
}

void nyc_bench(void)
{
  std::vector<level_t> start_bits;
  std::vector<level_t> bit_widths;
  use_nyc_setting(NYC_DIMENSION, micro_nyc_size, bit_widths, start_bits);
  md_trie<NYC_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node, bit_widths, start_bits);
  MdTrieBench<NYC_DIMENSION> bench(&mdtrie);
  std::string folder_name = "microbenchmark/";
  if (identification_string != "")
  {
    folder_name = "optimization/";
  }
  bench.insert(NYC_DATA_ADDR,
               folder_name + "nyc_insert" + identification_string,
               total_points_count,
               parse_line_nyc);
  bench.get_storage(folder_name + "nyc_storage" + identification_string);
  bench.lookup(folder_name + "nyc_lookup" + identification_string);
  bench.range_search(NYC_QUERY_ADDR,
                     folder_name + "nyc_query" + identification_string,
                     get_query_nyc<NYC_DIMENSION>);
}

void tpch_bench(void)
{
  std::vector<level_t> start_bits;
  std::vector<level_t> bit_widths;
  use_tpch_setting(TPCH_DIMENSION, micro_tpch_size, bit_widths, start_bits);
  md_trie<TPCH_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node, bit_widths, start_bits);
  MdTrieBench<TPCH_DIMENSION> bench(&mdtrie);
  std::string folder_name = "microbenchmark/";
  if (identification_string != "")
  {
    folder_name = "optimization/";
  }
  bench.insert(TPCH_DATA_ADDR,
               folder_name + "tpch_insert" + identification_string,
               total_points_count,
               parse_line_tpch);
  bench.get_storage(folder_name + "tpch_storage" + identification_string);
  bench.lookup(folder_name + "tpch_lookup" + identification_string);
  bench.range_search(TPCH_QUERY_ADDR,
                     folder_name + "tpch_query" + identification_string,
                     get_query_tpch<TPCH_DIMENSION>);
}

void sensitivity_num_dimensions_4(void)
{
  std::vector<level_t> start_bits;
  std::vector<level_t> bit_widths;
  use_tpch_setting(4, micro_tpch_size, bit_widths, start_bits);
  total_points_count /= 50;
  md_trie<4> mdtrie(max_depth, trie_depth, max_tree_node, bit_widths, start_bits);
  MdTrieBench<4> bench(&mdtrie);

  bench.insert(TPCH_DATA_ADDR,
               "sensitivity/tpch_insert_dimensions" + identification_string +
                   "_" + "4",
               total_points_count,
               parse_line_tpch);
  bench.lookup("sensitivity/tpch_lookup_dimensions" + identification_string +
               "_" + "4");
  bench.get_storage("sensitivity/tpch_storage_dimensions" +
                    identification_string + "_" + "4");
  bench.range_search_random("sensitivity/tpch_query_dimensions" +
                                identification_string + "_" + "4",
                            get_random_query_tpch<4>,
                            total_points_count,
                            1);
}

void sensitivity_num_dimensions_8(void)
{
  std::vector<level_t> start_bits;
  std::vector<level_t> bit_widths;
  use_tpch_setting(8, micro_tpch_size, bit_widths, start_bits);
  total_points_count /= 50;
  md_trie<8> mdtrie(max_depth, trie_depth, max_tree_node, bit_widths, start_bits);
  MdTrieBench<8> bench(&mdtrie);

  bench.insert(TPCH_DATA_ADDR,
               "sensitivity/tpch_insert_dimensions" + identification_string +
                   "_" + "8",
               total_points_count,
               parse_line_tpch);
  bench.lookup("sensitivity/tpch_lookup_dimensions" + identification_string +
               "_" + "8");
  bench.get_storage("sensitivity/tpch_storage_dimensions" +
                    identification_string + "_" + "8");
  bench.range_search_random("sensitivity/tpch_query_dimensions" +
                                identification_string + "_" + "8",
                            get_random_query_tpch<8>,
                            total_points_count,
                            1);
}

void sensitivity_num_dimensions_16_mdtries(void)
{
  std::vector<level_t> start_bits;
  std::vector<level_t> bit_widths;
  use_tpch_setting(16, micro_tpch_size, bit_widths, start_bits);
  total_points_count /= 50;
  MdTries<16> mdtries(max_depth, trie_depth, max_tree_node, bit_widths, start_bits);
  MdTrieBench<16> bench(&mdtries);

  bench.insert(TPCH_DATA_ADDR,
               "sensitivity/tpch_insert_dimensions" + identification_string +
                   "_" + "16",
               total_points_count,
               parse_line_tpch);
  bench.lookup("sensitivity/tpch_lookup_dimensions" + identification_string +
               "_" + "16");
  bench.get_storage("sensitivity/tpch_storage_dimensions" +
                    identification_string + "_" + "16");
  bench.range_search_random("sensitivity/tpch_query_dimensions" +
                                identification_string + "_" + "16",
                            get_random_query_tpch<16>,
                            total_points_count,
                            1);
}

void sensitivity_num_dimensions_32_mdtries(void)
{
  std::vector<level_t> start_bits;
  std::vector<level_t> bit_widths;
  use_tpch_setting(32, micro_tpch_size, bit_widths, start_bits);
  total_points_count /= 50;
  MdTries<32> mdtries(max_depth, trie_depth, max_tree_node, bit_widths, start_bits);
  MdTrieBench<32> bench(&mdtries);

  bench.insert(TPCH_DATA_ADDR,
               "sensitivity/tpch_insert_dimensions" + identification_string +
                   "_" + "32",
               total_points_count,
               parse_line_tpch);
  bench.lookup("sensitivity/tpch_lookup_dimensions" + identification_string +
               "_" + "32");
  bench.get_storage("sensitivity/tpch_storage_dimensions" +
                    identification_string + "_" + "32");
  bench.range_search_random("sensitivity/tpch_query_dimensions" +
                                identification_string + "_" + "32",
                            get_random_query_tpch<32>,
                            total_points_count,
                            1);
}

void sensitivity_num_dimensions_64_mdtries(void)
{
  std::vector<level_t> start_bits;
  std::vector<level_t> bit_widths;
  use_tpch_setting(64, micro_tpch_size, bit_widths, start_bits);
  total_points_count /= 50;
  MdTries<64> mdtries(max_depth, trie_depth, max_tree_node, bit_widths, start_bits);
  MdTrieBench<64> bench(&mdtries);

  bench.insert(TPCH_DATA_ADDR,
               "sensitivity/tpch_insert_dimensions" + identification_string +
                   "_" + "64",
               total_points_count,
               parse_line_tpch);
  bench.lookup("sensitivity/tpch_lookup_dimensions" + identification_string +
               "_" + "64");
  bench.get_storage("sensitivity/tpch_storage_dimensions" +
                    identification_string + "_" + "64");
  bench.range_search_random("sensitivity/tpch_query_dimensions" +
                                identification_string + "_" + "64",
                            get_random_query_tpch<64>,
                            total_points_count,
                            1);
}

void sensitivity_num_dimensions_128_mdtries(void)
{
  std::vector<level_t> start_bits;
  std::vector<level_t> bit_widths;
  use_tpch_setting(128, micro_tpch_size, bit_widths, start_bits);
  total_points_count /= 50;
  MdTries<128> mdtries(max_depth, trie_depth, max_tree_node, bit_widths, start_bits);
  MdTrieBench<128> bench(&mdtries);

  bench.insert(TPCH_DATA_ADDR,
               "sensitivity/tpch_insert_dimensions" + identification_string +
                   "_" + "128",
               total_points_count,
               parse_line_tpch);
  bench.lookup("sensitivity/tpch_lookup_dimensions" + identification_string +
               "_" + "128");
  bench.get_storage("sensitivity/tpch_storage_dimensions" +
                    identification_string + "_" + "128");
  bench.range_search_random("sensitivity/tpch_query_dimensions" +
                                identification_string + "_" + "128",
                            get_random_query_tpch<128>,
                            total_points_count,
                            1);
}

void sensitivity_selectivity(void)
{
  std::vector<level_t> start_bits;
  std::vector<level_t> bit_widths;
  use_tpch_setting(TPCH_DIMENSION, micro_tpch_size, bit_widths, start_bits);
  md_trie<TPCH_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node, bit_widths, start_bits);
  MdTrieBench<TPCH_DIMENSION> bench(&mdtrie);

  bench.insert(TPCH_DATA_ADDR,
               "sensitivity/tpch_insert_selectivity" + identification_string,
               total_points_count,
               parse_line_tpch);
  bench.range_search_random("sensitivity/tpch_query_selectivity" +
                                identification_string,
                            get_random_query_tpch<TPCH_DIMENSION>,
                            total_points_count,
                            1);
}

void sensitivity_treeblock_sizes(int treeblock_size)
{
  std::vector<level_t> start_bits;
  std::vector<level_t> bit_widths;
  use_tpch_setting(TPCH_DIMENSION, micro_tpch_size, bit_widths, start_bits);
  md_trie<TPCH_DIMENSION> mdtrie(max_depth, trie_depth, treeblock_size, bit_widths, start_bits);
  MdTrieBench<TPCH_DIMENSION> bench(&mdtrie);
  bench.insert(TPCH_DATA_ADDR,
               "sensitivity/tpch_insert_treeblock_sizes_" + std::to_string(treeblock_size),
               total_points_count,
               parse_line_tpch);

  bench.get_storage("sensitivity/tpch_storage_treeblock_sizes_" + std::to_string(treeblock_size));
  bench.lookup("sensitivity/tpch_lookup_treeblock_sizes_" + std::to_string(treeblock_size));
  bench.range_search(TPCH_QUERY_ADDR,
                     "sensitivity/tpch_query_treeblock_sizes_" + std::to_string(treeblock_size),
                     get_query_tpch<TPCH_DIMENSION>);
}

int main(int argc, char *argv[])
{

  std::string argvalue;
  optimization_code = OPTIMIZATION_SM;
  int arg;
  int sensitivity_dimensions = -1;
  int treeblock_size = -1;
  is_microbenchmark = true;

  while ((arg = getopt(argc, argv, "b:o:d:t:")) != -1)
    switch (arg)
    {
    case 'b':
      argvalue = std::string(optarg);
      break;
    case 't':
      treeblock_size = atoi(optarg);
      break;
    case 'o':
      optimization = std::string(optarg);
      if (optimization == "SM")
        optimization_code = OPTIMIZATION_SM;
      if (optimization == "B")
        optimization_code = OPTIMIZATION_B;
      if (optimization == "CN")
        optimization_code = OPTIMIZATION_CN;
      if (optimization == "GM")
        optimization_code = OPTIMIZATION_GM;
      break;
    case 'd':
      sensitivity_dimensions = atoi(optarg);
      break;
    default:
      abort();
    }

  std::cout << "benchmark: " << argvalue << ", optimization: " << optimization
            << ", dimensions: " << sensitivity_dimensions
            << ", treeblock_size: " << treeblock_size << std::endl;
  if (argvalue == "tpch")
    tpch_bench();
  else if (argvalue == "github")
    github_bench();
  else if (argvalue == "nyc")
    nyc_bench();
  else if (argvalue == "sensitivity_num_dimensions")
  {
    switch (sensitivity_dimensions)
    {
    case 128:
      sensitivity_num_dimensions_128_mdtries();
      break;
    case 64:
      sensitivity_num_dimensions_64_mdtries();
      break;
    case 32:
      sensitivity_num_dimensions_32_mdtries();
      break;
    case 16:
      sensitivity_num_dimensions_16_mdtries();
      break;
    case 8:
      sensitivity_num_dimensions_8();
      break;
    case 4:
      sensitivity_num_dimensions_4();
      break;
    default:
      std::cout << "wrong dimensions: " << sensitivity_dimensions
                << std::endl;
      abort();
    }
  }
  else if (argvalue == "sensitivity_selectivity")
    sensitivity_selectivity();
  else if (argvalue == "sensitivity_treeblock_size")
    sensitivity_treeblock_sizes(treeblock_size);
  else
    std::cout << "Unrecognized benchmark: " << argvalue << std::endl;
}