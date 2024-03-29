#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "MDTrieShardClient.h"
#include "benchmark.hpp"
#include "trie.h"
#include <atomic>
#include <fstream>
#include <future>
#include <iostream>
#include <istream>
#include <streambuf>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <tuple>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

int main(int argc, char *argv[])
{

  int dataset_part;
  std::string benchmark_str;
  int arg;
  uint32_t throughput;

  while ((arg = getopt(argc, argv, "b:d:")) != -1)
  {

    switch (arg)
    {
    case 'd':
      dataset_part = stoi(optarg);
      break;
    case 'b':
      benchmark_str = std::string(optarg);
      break;
    default:
      abort();
    }
  }

  std::ofstream outfile(results_folder_addr + "macrobenchmark/github/github_" +
                            std::to_string(dataset_part),
                        ios_base::app);

  std::vector<std::string> server_ips = {
      "10.10.1.12", "10.10.1.13", "10.10.1.14", "10.10.1.15", "10.10.1.16"};
  TrinityBench bench(shard_num,
                     client_num,
                     client_server_number,
                     server_ips,
                     GITHUB,
                     dataset_part);

  if (benchmark_str == "insert")
  {
    throughput = bench.insert_benchmark();
    outfile << dataset_part
            << ": Insert Throughput (pt / seconds): " << throughput
            << std::endl;
    benchmark_str = "storage";
    std::ofstream outfile_storage(results_folder_addr +
                                      "macrobenchmark/github/github_" + benchmark_str,
                                  ios_base::app);
    outfile_storage << dataset_part << ": size: " << bench.get_size() << ","
                    << bench.get_dataset_size() << std::endl; /* So that storage
                       doesn't need to load it again */
  }
  else if (benchmark_str == "storage")
  {
    outfile << dataset_part << ": size: " << bench.get_size() << ","
            << bench.get_dataset_size() << std::endl;
  }
  else if (benchmark_str == "lookup")
  {
    throughput = bench.lookup_benchmark();
    outfile << dataset_part
            << ": Lookup Throughput (pt / seconds): " << throughput
            << std::endl;
  }
  else if (benchmark_str == "query_latency")
  {
    bench.search_latency_benchmark();
  }
  else if (benchmark_str == "query_throughput")
  {
    throughput = bench.search_benchmark();
    outfile << dataset_part
            << ": Search Throughput (pt / seconds): " << throughput
            << std::endl;
  }
  /* 95% lookup, 5% insert*/
  else if (benchmark_str == "lookup_mixed")
  {
    throughput = bench.mixed_query_benchmark(5, 95, 0);
    outfile << dataset_part
            << ": 95 lookup, 5 insert Throughput (pt / seconds): " << throughput
            << std::endl;
  }
  /* 95% search, 5% insert*/
  else if (benchmark_str == "search_mixed")
  {
    throughput = bench.mixed_query_benchmark(1, 0, 20);
    outfile << dataset_part
            << ": 95 search, 5 insert Throughput (pt / seconds): " << throughput
            << std::endl;
  }
  /* 90% insert, 10% search */
  else if (benchmark_str == "insert_mixed")
  {
    throughput = bench.mixed_query_benchmark(9, 0, 1);
    outfile << dataset_part
            << ": 10 search, 90 insert Throughput (pt / seconds): "
            << throughput << std::endl;
  }
  else
  {
    outfile << "No benchmark: " << benchmark_str << std::endl;
  }
}