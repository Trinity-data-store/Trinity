#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "MDTrieShardClient.h"
#include "trie.h"
#include <future>
#include <atomic>
#include <tuple>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <istream>
#include <sys/stat.h>
#include <sys/mman.h>
#include "benchmark.hpp"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

int main(int argc, char *argv[]){

    int dataset_part;
    std::string benchmark_str;
    int arg;
    uint32_t throughput;
    
    while ((arg = getopt (argc, argv, "b:d:")) != -1) {

        switch (arg) {
            case 'd':
                dataset_part = stoi(optarg);
                break;
            case 'b':
                benchmark_str = std::string(optarg);
                break;
            default:
                abort ();
        }
    }

    std::ofstream outfile(results_folder_addr + "macrobenchmark/tpch_macro_Trinity_" + std::to_string(dataset_part), ios_base::app);

    std::vector<std::string> server_ips = {"10.10.1.12", "10.10.1.13", "10.10.1.14", "10.10.1.15", "10.10.1.16"};
    TrinityBench bench(shard_num, client_num, client_server_number, server_ips, TPCH, dataset_part);

    if (benchmark_str == "insert") {
        throughput = bench.insert_benchmark();
        outfile << dataset_part << ": Insert Throughput (pt / seconds): " << throughput << std::endl; 
        outfile << "size: " << bench.get_size() << std::endl; /* So that storage doesn't need to load it again */
    }   
    else if (benchmark_str == "storage") {
        outfile << "size: " << bench.get_size() << "," << TPCH_SIZE << std::endl;
    }
    else if (benchmark_str == "lookup") {
        throughput = bench.lookup_benchmark();
        outfile << dataset_part << ": Lookup Throughput (pt / seconds): " << throughput << std::endl;    
    }
    else if (benchmark_str == "query_latency") {
        bench.search_latency_benchmark();
    }
    else if (benchmark_str == "query_throughput") {
        throughput = bench.search_benchmark();
        outfile << dataset_part << ": Search Throughput (pt / seconds): " << throughput << std::endl;    
    }
    /* 95% lookup, 5% insert*/
    else if (benchmark_str == "lookup_mixed") {
        throughput = bench.mixed_query_benchmark(5, 95, 0);
        outfile << dataset_part << ": 95 lookup, 5 insert Throughput (pt / seconds): " << throughput << std::endl;    
    }
    /* 95% search, 5% insert*/
    else if (benchmark_str == "search_mixed") {
        throughput = bench.mixed_query_benchmark(1, 0, 20);
        outfile << dataset_part << ": 95 search, 5 insert Throughput (pt / seconds): " << throughput << std::endl;    
    }
    /* 90% insert, 10% search */
    else if (benchmark_str == "insert_mixed") {
        throughput = bench.mixed_query_benchmark(9, 0, 1);
        outfile << dataset_part << ": 10 search, 90 insert Throughput (pt / seconds): " << throughput << std::endl; 
    }   
    else {
        outfile << "No benchmark: " << benchmark_str << std::endl;
    }
}