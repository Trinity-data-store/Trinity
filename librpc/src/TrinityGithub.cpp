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

const int shard_num = 20;
const int client_num = 100;
const int client_server_number = 10;

int main(int argc, char *argv[]){

    if (argc != 2) {
        cerr << "./TrinityGithub [dataset part]" << std::endl;
        exit(-1);
    }
    int dataset_part = stoi(argv[1]);

    std::vector<std::string> server_ips = {"10.10.1.12", "10.10.1.13", "10.10.1.14", "10.10.1.15", "10.10.1.16"};
    TrinityBench bench(shard_num, client_num, client_server_number, server_ips, GITHUB, dataset_part);

    uint32_t throughput = bench.insert_benchmark();
    std::cout << dataset_part << ": Insert Throughput (pt / seconds): " << throughput << std::endl;    

    std::cout << dataset_part << ": size: " << bench.get_size() << std::endl;

    throughput = bench.lookup_benchmark();
    std::cout << dataset_part << ": Lookup Throughput (pt / seconds): " << throughput << std::endl;    

    bench.search_latency_benchmark();

    throughput = bench.search_benchmark();
    std::cout << dataset_part << ": Search Throughput (pt / seconds): " << throughput << std::endl;    

    /* 95% lookup, 5% insert*/
    throughput = bench.mixed_query_benchmark(5, 95, 0);
    std::cout << dataset_part << ": 95 lookup, 5 insert Throughput (pt / seconds): " << throughput << std::endl;    

    /* 95% search, 5% insert*/
    throughput = bench.mixed_query_benchmark(5, 0, 95);
    std::cout << dataset_part << ": 95 search, 5 insert Throughput (pt / seconds): " << throughput << std::endl;    

    /* 90% insert, 10% search */
    throughput = bench.mixed_query_benchmark(90, 0, 10);
    std::cout << dataset_part << ": 10 search, 90 insert Throughput (pt / seconds): " << throughput << std::endl;    
}