#include "benchmark.hpp"
#include "common.hpp"
#include "parser.hpp"
#include "trie.h"
#include <climits>
#include <fstream>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

void
github_bench(void)
{

  use_github_setting(GITHUB_DIMENSION, micro_github_size);
  md_trie<GITHUB_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
  MdTrieBench<GITHUB_DIMENSION> bench(&mdtrie);
  p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);

  bench.insert(GITHUB_DATA_ADDR,
               "github_insert_micro" + identification_string,
               total_points_count,
               parse_line_github);
  bench.get_storage("github_storage_micro" + identification_string);
  bench.lookup("github_lookup_micro" + identification_string);
  bench.range_search(GITHUB_QUERY_ADDR,
                     "github_query_micro" + identification_string,
                     get_query_github<GITHUB_DIMENSION>);
}

void
nyc_bench(void)
{

  use_nyc_setting(NYC_DIMENSION, micro_nyc_size);
  md_trie<NYC_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
  MdTrieBench<NYC_DIMENSION> bench(&mdtrie);
  p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);

  bench.insert(NYC_DATA_ADDR,
               "nyc_insert_micro" + identification_string,
               total_points_count,
               parse_line_nyc);
  bench.get_storage("nyc_storage_micro" + identification_string);
  bench.lookup("nyc_lookup_micro" + identification_string);
  bench.range_search(NYC_QUERY_ADDR,
                     "nyc_query_micro" + identification_string,
                     get_query_nyc<NYC_DIMENSION>);
}

void
tpch_bench(void)
{

  use_tpch_setting(TPCH_DIMENSION, micro_tpch_size);
  md_trie<TPCH_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
  MdTrieBench<TPCH_DIMENSION> bench(&mdtrie);
  p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);

  bench.insert(TPCH_DATA_ADDR,
               "tpch_insert_micro" + identification_string,
               total_points_count,
               parse_line_tpch);
  bench.get_storage("tpch_storage_micro" + identification_string);
  bench.lookup("tpch_lookup_micro" + identification_string);
  bench.range_search(TPCH_QUERY_ADDR,
                     "tpch_query_micro" + identification_string,
                     get_query_tpch<TPCH_DIMENSION>);
}

void
sensitivity_num_dimensions_9(void)
{

  use_tpch_setting(9, micro_tpch_size);
  md_trie<9> mdtrie(max_depth, trie_depth, max_tree_node);
  MdTrieBench<9> bench(&mdtrie);
  p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);

  bench.insert(TPCH_DATA_ADDR,
               "tpch_insert_dimensions_sensitivity" + identification_string +
                 "_" + "9",
               total_points_count,
               parse_line_tpch);
  bench.get_storage("tpch_storage_dimensions_sensitivity" +
                    identification_string + "_" + "9");
  bench.range_search_random("tpch_query_dimensions_sensitivity" +
                              identification_string + "_" + "9",
                            get_random_query_tpch<9>,
                            micro_tpch_size * selectivity_upper,
                            micro_tpch_size * selectivity_lower);
}

void
sensitivity_num_dimensions_8(void)
{

  use_tpch_setting(8, micro_tpch_size);
  md_trie<8> mdtrie(max_depth, trie_depth, max_tree_node);
  MdTrieBench<8> bench(&mdtrie);
  p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);

  bench.insert(TPCH_DATA_ADDR,
               "tpch_insert_dimensions_sensitivity" + identification_string +
                 "_" + "8",
               total_points_count,
               parse_line_tpch);
  bench.get_storage("tpch_storage_dimensions_sensitivity" +
                    identification_string + "_" + "8");
  bench.range_search_random("tpch_query_dimensions_sensitivity" +
                              identification_string + "_" + "8",
                            get_random_query_tpch<8>,
                            micro_tpch_size * selectivity_upper,
                            micro_tpch_size * selectivity_lower);
}

void
sensitivity_num_dimensions_7(void)
{

  use_tpch_setting(7, micro_tpch_size);
  md_trie<7> mdtrie(max_depth, trie_depth, max_tree_node);
  MdTrieBench<7> bench(&mdtrie);
  p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);

  bench.insert(TPCH_DATA_ADDR,
               "tpch_insert_dimensions_sensitivity" + identification_string +
                 "_" + "7",
               total_points_count,
               parse_line_tpch);
  bench.get_storage("tpch_storage_dimensions_sensitivity" +
                    identification_string + "_" + "7");
  bench.range_search_random("tpch_query_dimensions_sensitivity" +
                              identification_string + "_" + "7",
                            get_random_query_tpch<7>,
                            micro_tpch_size * selectivity_upper,
                            micro_tpch_size * selectivity_lower);
}

void
sensitivity_num_dimensions_6(void)
{

  use_tpch_setting(6, micro_tpch_size);
  md_trie<6> mdtrie(max_depth, trie_depth, max_tree_node);
  MdTrieBench<6> bench(&mdtrie);
  p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);

  bench.insert(TPCH_DATA_ADDR,
               "tpch_insert_dimensions_sensitivity" + identification_string +
                 "_" + "6",
               total_points_count,
               parse_line_tpch);
  bench.get_storage("tpch_storage_dimensions_sensitivity" +
                    identification_string + "_" + "6");
  bench.range_search_random("tpch_query_dimensions_sensitivity" +
                              identification_string + "_" + "6",
                            get_random_query_tpch<6>,
                            micro_tpch_size * selectivity_upper,
                            micro_tpch_size * selectivity_lower);
}

void
sensitivity_num_dimensions_5(void)
{

  use_tpch_setting(5, micro_tpch_size);
  md_trie<5> mdtrie(max_depth, trie_depth, max_tree_node);
  MdTrieBench<5> bench(&mdtrie);
  p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);

  bench.insert(TPCH_DATA_ADDR,
               "tpch_insert_dimensions_sensitivity" + identification_string +
                 "_" + "5",
               total_points_count,
               parse_line_tpch);
  bench.get_storage("tpch_storage_dimensions_sensitivity" +
                    identification_string + "_" + "5");
  bench.range_search_random("tpch_query_dimensions_sensitivity" +
                              identification_string + "_" + "5",
                            get_random_query_tpch<5>,
                            micro_tpch_size * selectivity_upper,
                            micro_tpch_size * selectivity_lower);
}

void
sensitivity_num_dimensions_4(void)
{

  use_tpch_setting(4, micro_tpch_size);
  md_trie<4> mdtrie(max_depth, trie_depth, max_tree_node);
  MdTrieBench<4> bench(&mdtrie);
  p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);

  bench.insert(TPCH_DATA_ADDR,
               "tpch_insert_dimensions_sensitivity" + identification_string +
                 "_" + "4",
               total_points_count,
               parse_line_tpch);
  bench.get_storage("tpch_storage_dimensions_sensitivity" +
                    identification_string + "_" + "4");
  bench.range_search_random("tpch_query_dimensions_sensitivity" +
                              identification_string + "_" + "4",
                            get_random_query_tpch<4>,
                            micro_tpch_size * selectivity_upper,
                            micro_tpch_size * selectivity_lower);
}

void
sensitivity_selectivity(void)
{

  use_tpch_setting(TPCH_DIMENSION, micro_tpch_size);
  md_trie<TPCH_DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
  MdTrieBench<TPCH_DIMENSION> bench(&mdtrie);
  p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);

  bench.insert(TPCH_DATA_ADDR,
               "tpch_insert_selectivity_sensitivity" + identification_string,
               total_points_count,
               parse_line_tpch);
  bench.range_search_random("tpch_query_selectivity_sensitivity" +
                              identification_string,
                            get_random_query_tpch<TPCH_DIMENSION>,
                            total_points_count,
                            1);
}

int
main(int argc, char* argv[])
{

  std::string argvalue;
  optimization_code = OPTIMIZATION_SM;
  int arg;
  int sensitivity_dimensions = -1;
  is_microbenchmark = true;

  while ((arg = getopt(argc, argv, "b:o:d:")) != -1)
    switch (arg) {
      case 'b':
        argvalue = std::string(optarg);
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
        switch (atoi(optarg)) {
          case 9:
            sensitivity_dimensions = 9;
            break;
          case 8:
            sensitivity_dimensions = 8;
            break;
          case 7:
            sensitivity_dimensions = 7;
            break;
          case 6:
            sensitivity_dimensions = 6;
            break;
          case 5:
            sensitivity_dimensions = 5;
            break;
          case 4:
            sensitivity_dimensions = 4;
            break;
          default:
            std::cout << "wrong dimensions: " << atoi(optarg) << std::endl;
            abort();
        }
        break;
      default:
        abort();
    }

  std::cout << "benchmark: " << argvalue << ", optimization: " << optimization
            << ", dimensions: " << sensitivity_dimensions << std::endl;
  if (argvalue == "tpch")
    tpch_bench();
  else if (argvalue == "github")
    github_bench();
  else if (argvalue == "nyc")
    nyc_bench();
  else if (argvalue == "sensitivity_num_dimensions") {
    switch (sensitivity_dimensions) {
      case 9:
        sensitivity_num_dimensions_9();
        break;
      case 8:
        sensitivity_num_dimensions_8();
        break;
      case 7:
        sensitivity_num_dimensions_7();
        break;
      case 6:
        sensitivity_num_dimensions_6();
        break;
      case 5:
        sensitivity_num_dimensions_5();
        break;
      case 4:
        sensitivity_num_dimensions_4();
        break;
      default:
        std::cout << "wrong dimensions: " << sensitivity_dimensions
                  << std::endl;
        abort();
    }
  } else if (argvalue == "sensitivity_selectivity")
    sensitivity_selectivity();
  else
    std::cout << "Unrecognized benchmark: " << argvalue << std::endl;
}