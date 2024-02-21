#ifndef TrinityBench_H
#define TrinityBench_H

#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "../../libmdtrie/bench/common.hpp"
#include "../../libmdtrie/bench/parser.hpp"
#include "MDTrieShardClient.h"
#include "trie.h"
#include <climits>
#include <fstream>
#include <future>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

const int shard_num = 20;
const int client_num = 100;
const int client_server_number = 10;

class TrinityBench
{
public:
  TrinityBench(int shard_num,
               int client_num,
               int client_server_num,
               std::vector<std::string> server_ips,
               int dataset_idx,
               int dataset_part)
  {
    shard_num_ = shard_num;
    client_num_ = client_num;
    server_ips_ = server_ips;
    dataset_part_ = dataset_part;
    client_server_num_ = client_server_num;
    primary_key_offset_ = 0; /* This is for mixed query workload, to ensure that
                                same insertion does not go to the same shard */
    dataset_idx_ = dataset_idx;

    auto client = MDTrieClient(server_ips_, shard_num_);
    if (!client.ping(dataset_idx))
    {
      std::cerr << "Server setting wrong!" << std::endl;
      exit(-1);
    }

    switch (dataset_idx)
    {
    case (TPCH):
      dataset_size_ = TPCH_SIZE / client_server_num_;
      dataset_addr_ = TPCH_SPLIT_ADDR + "x0" + std::to_string(dataset_part);
      query_addr_ = TPCH_QUERY_ADDR;
      num_dimensions_ = TPCH_DIMENSION;
      max_values_ = tpch_max_values;
      min_values_ = tpch_min_values;
      query_out_addr_ =
          results_folder_addr + "macrobenchmark/tpch/tpch_query_latency";
      load_dataset(parse_line_tpch);
      load_queries(update_range_search_range_tpch);
      break;
    case (GITHUB):
      dataset_size_ = GITHUB_SIZE / client_server_num_;
      dataset_addr_ = GITHUB_SPLIT_ADDR + "x0" + std::to_string(dataset_part);
      query_addr_ = GITHUB_QUERY_ADDR;
      num_dimensions_ = GITHUB_DIMENSION;
      max_values_ = github_max_values;
      min_values_ = github_min_values;
      query_out_addr_ =
          results_folder_addr + "macrobenchmark/github/github_query_latency";
      load_dataset(parse_line_github);
      load_queries(update_range_search_range_github);
      break;
    case (NYC):
      dataset_size_ = NYC_SIZE / client_server_num_;
      dataset_addr_ = NYC_SPLIT_ADDR + "x0" + std::to_string(dataset_part);
      query_addr_ = NYC_QUERY_ADDR;
      num_dimensions_ = NYC_DIMENSION;
      max_values_ = nyc_max_values;
      min_values_ = nyc_min_values;
      query_out_addr_ =
          results_folder_addr + "macrobenchmark/nyc/nyc_query_latency";
      load_dataset(parse_line_nyc);
      load_queries(update_range_search_range_nyc);
      break;
    default:
      std::cerr << "Unrecognized dataset idx: " << dataset_idx << std::endl;
      exit(-1);
    }

    warmup_size_ = dataset_size_ / 100;
  };

  uint32_t insert_benchmark(void)
  {

    std::vector<std::future<uint32_t>> threads;
    threads.reserve(client_num_);

    std::cout << "Started warmup: " << warmup_size_ << std::endl;
    this->warmup();
    std::cout << "Finished warmup" << std::endl;

    for (int i = 0; i < client_num_; i++)
    {
      threads.push_back(
          std::async(&TrinityBench::insert_each_client, this, i, dataset_size_));
    }

    uint32_t total_throughput = 0;
    for (int i = 0; i < client_num_; i++)
    {
      total_throughput += threads[i].get();
    }
    std::cout << "Finished insert_benchmark" << std::endl;
    return total_throughput;
  }

  uint32_t lookup_benchmark(void)
  {

    std::vector<std::future<uint32_t>> threads;
    threads.reserve(client_num_);

    for (int i = 0; i < client_num_; i++)
    {
      threads.push_back(
          std::async(&TrinityBench::lookup_each_client, this, i, dataset_size_));
    }

    uint32_t total_throughput = 0;
    for (int i = 0; i < client_num_; i++)
    {
      total_throughput += threads[i].get();
    }
    std::cout << "Finished lookup_benchmark" << std::endl;
    return total_throughput;
  }

  uint32_t search_benchmark(void)
  {

    int search_client_num = 5;
    std::vector<std::future<uint32_t>> threads;
    threads.reserve(search_client_num);

    for (int i = 0; i < search_client_num; i++)
    {
      threads.push_back(std::async(&TrinityBench::search_each_client, this, i));
    }

    uint32_t total_throughput = 0;
    for (int i = 0; i < search_client_num; i++)
    {
      total_throughput += threads[i].get();
    }
    std::cout << "Finished search_benchmark" << std::endl;
    return total_throughput;
  }

  uint32_t mixed_query_benchmark(int num_insert, int num_lookup, int num_search)
  {

    std::vector<std::future<uint32_t>> threads;
    threads.reserve(client_num_);
    primary_key_offset_ = 1;
    uint32_t benchmark_count_stopped = dataset_size_ / client_num_;
    for (int i = 0; i < num_insert + num_lookup + num_search; i++)
    {
      threads.push_back(std::async(&TrinityBench::mixed_query_each_client,
                                   this,
                                   i,
                                   num_insert,
                                   num_lookup,
                                   num_search,
                                   benchmark_count_stopped));
    }

    uint32_t total_throughput = 0;
    for (int i = 0; i < num_lookup + num_search + num_insert; i++)
    {
      total_throughput += threads[i].get();
    }
    primary_key_offset_ = 0;
    std::cout << "Finished mixed_query_benchmark" << std::endl;
    return total_throughput;
  }

  uint32_t get_size(void)
  {
    auto client = MDTrieClient(server_ips_, shard_num_);
    return client.get_size();
  }

  void search_latency_benchmark(void)
  {

    auto client = MDTrieClient(server_ips_, shard_num_);
    TimeStamp start, diff = 0;
    start = GetTimestamp();

    std::ofstream outFile(query_out_addr_);

    for (int i = 0; i < QUERY_NUM; i++)
    {
      std::vector<int32_t> found_points;
      start = GetTimestamp();
      client.range_search_trie(
          found_points, queries_start_vect_[i], queries_end_vect_[i]);
      diff = GetTimestamp() - start;
      outFile << "Query " << i << " end to end latency (ms): " << diff / 1000
              << ", found points count: "
              << found_points.size() << std::endl;
    }
  }

  uint32_t get_dataset_size(void) { return dataset_size_; }

protected:
  void load_dataset(std::vector<int32_t> (*parse_line)(std::string))
  {
    std::cout << "Start load_dataset" << std::endl;
    std::ifstream file(dataset_addr_);
    std::string line;
    unsigned int loaded_pts = 0;
    while (std::getline(file, line))
    {
      points_vect_.push_back(parse_line(line));
      loaded_pts++;
      if (loaded_pts > dataset_size_)
        break;
    }
    dataset_size_ = loaded_pts;
    std::cout << "dataset_addr:" << dataset_addr_ << ", Dataset size: " << dataset_size_ << std::endl;
  }

  void load_queries(void (*parse_query)(std::vector<int32_t> &,
                                        std::vector<int32_t> &,
                                        std::string))
  {

    std::ifstream file(query_addr_);
    std::string line;
    std::vector<int32_t> start_range = min_values_;
    std::vector<int32_t> end_range = max_values_;

    while (std::getline(file, line))
    {
      parse_query(start_range, end_range, line);
      queries_start_vect_.push_back(start_range);
      queries_end_vect_.push_back(end_range);
      start_range = min_values_;
      end_range = max_values_;
    }
  }

  uint32_t insert_each_client(int client_index, uint32_t points_to_insert)
  {

    auto client = MDTrieClient(server_ips_, shard_num_);
    uint32_t inserted_points_count = 0;
    TimeStamp start, diff = 0;

    start = GetTimestamp();
    for (uint32_t point_idx = warmup_size_ + client_index;
         point_idx < points_to_insert;
         point_idx += client_num_)
    {

      vector<int32_t> data_point = points_vect_[point_idx];
      client.insert(data_point, point_idx + primary_key_offset_);
      inserted_points_count++;
    }
    diff = GetTimestamp() - start;
    return ((float)inserted_points_count / diff) * 1000000;
  }

  void warmup(void)
  {

    auto client = MDTrieClient(server_ips_, shard_num_);

    for (uint32_t point_idx = 0; point_idx < warmup_size_; point_idx++)
    {
      vector<int32_t> data_point = points_vect_[point_idx];
      client.insert(data_point, point_idx);
    }
  }

  uint32_t lookup_each_client(int client_index, uint32_t points_to_lookup)
  {

    auto client = MDTrieClient(server_ips_, shard_num_);
    uint32_t lookup_points_count = 0;
    TimeStamp start, diff = 0;

    start = GetTimestamp();
    for (uint32_t point_idx = client_index; point_idx < points_to_lookup;
         point_idx += client_num_)
    {
      std::vector<int32_t> rec_vect;
      client.primary_key_lookup(rec_vect, point_idx);
      lookup_points_count++;
    }
    diff = GetTimestamp() - start;
    return ((float)lookup_points_count / diff) * 1000000;
  }

  uint32_t search_each_client(int client_index)
  {

    auto client = MDTrieClient(server_ips_, shard_num_);
    TimeStamp start, diff = 0;
    uint32_t search_points_count = 0;
    start = GetTimestamp();

    for (int i = client_index; i < QUERY_NUM; i += client_num_)
    {

      std::vector<int32_t> found_points;
      client.range_search_trie_send(queries_start_vect_[i],
                                    queries_end_vect_[i]);
      client.range_search_trie_rec(found_points);

      search_points_count += found_points.size();
    }
    diff = GetTimestamp() - start;
    return ((float)search_points_count / diff) * 1000000;
  }

  uint32_t mixed_query_each_client(int client_index,
                                   int num_insert,
                                   int num_lookup,
                                   int num_search,
                                   uint32_t benchmark_count_stopped)
  {

    auto client = MDTrieClient(server_ips_, shard_num_);
    TimeStamp start, diff = 0;
    uint32_t benchmark_count =
        client_index; /* Each client starts from a different position */
    int num_insert_lookup_search = num_insert + num_lookup + num_search;

    std::cout << "mixed_query_each_client: " << client_index << ","
              << num_insert << "," << num_lookup << "," << num_search << ","
              << benchmark_count_stopped << std::endl;
    start = GetTimestamp();

    while (benchmark_count < benchmark_count_stopped)
    {

      int random_num = gen_rand(0, num_insert_lookup_search - 1);
      if (random_num % num_insert_lookup_search < num_insert)
      {
        /* Insert */
        int point_idx = benchmark_count % dataset_size_;
        vector<int32_t> data_point = points_vect_[point_idx];
        client.insert(data_point, point_idx + primary_key_offset_);
        benchmark_count++;
      }
      else if (random_num % num_insert_lookup_search <
               num_insert + num_lookup)
      {
        /* Lookup */
        int point_idx = benchmark_count % dataset_size_;
        std::vector<int32_t> rec_vect;
        client.primary_key_lookup(rec_vect, point_idx);
        benchmark_count++;
      }
      else
      {
        /* Search*/
        std::vector<int32_t> found_points;
        client.range_search_trie_send(
            queries_start_vect_[benchmark_count % QUERY_NUM],
            queries_end_vect_[benchmark_count % QUERY_NUM]);
        client.range_search_trie_rec(found_points);
        benchmark_count += found_points.size();
      }
    }
    diff = GetTimestamp() - start;
    return ((float)benchmark_count / diff) * 1000000;
  }

  int shard_num_;
  int client_num_;
  int client_index_;
  int client_server_num_;
  int dataset_part_;
  int dataset_idx_;
  int primary_key_offset_;
  std::vector<std::string> server_ips_;
  uint32_t dataset_size_;
  uint32_t warmup_size_;
  int num_dimensions_;
  std::string dataset_addr_;
  std::string query_addr_;
  std::string query_out_addr_;
  std::vector<std::vector<int32_t>> points_vect_;
  std::vector<std::vector<int32_t>> queries_start_vect_;
  std::vector<std::vector<int32_t>> queries_end_vect_;
  std::vector<int32_t> max_values_;
  std::vector<int32_t> min_values_;
};

#endif // TrinityBench_H
