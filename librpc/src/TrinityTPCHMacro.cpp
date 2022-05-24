#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "MDTrieShardClient.h"
#include "TrinityBenchShared.h"
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

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
const int DIMENSION = 9;
const int shard_num = 20;
const int client_num = 20;

int main(int argc, char *argv[]){

    if (argc != 2) {
      cerr << "./TrinityTPCHMacro [dataset part]" << endl;
    }

    int which_part = stoi(argv[1]);

    std::vector<std::string> server_ips = {"10.254.254.225", "10.254.254.249", "10.254.254.205", "10.254.254.217", "10.254.254.209"};
    server_ips = {"10.10.1.5", "10.10.1.6", "10.10.1.7", "10.10.1.8", "10.10.1.9"};

    total_points_count = 3000028242;
    auto client = MDTrieClient(server_ips, shard_num);

    if (!client.ping(2)){
        std::cerr << "Server setting wrong!" << std::endl;
        exit(-1);
    }

    /** 
        Insert all points
    */

    TimeStamp start = GetTimestamp();
    uint32_t throughput = total_client_insert_split_file(shard_num, client_num, server_ips, which_part);
    TimeStamp diff = GetTimestamp() - start;

    cout << "Insertion Throughput (pt / seconds): " << throughput << endl;
    cout << "End-to-end Latency (s): " << diff / 1000000 << endl;
    cout << "Storage: " << client.get_size() << endl;
    
    // One client does query for simplicity
    if (which_part != 1) {
      return 0;
    }
    
    /** 
        Range Search
    */

    // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
    std::vector<int32_t> max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 59591284, 19980802};
    std::vector<int32_t> min_values = {1, 90001, 0, 0, 19920102, 19920131, 19920103, 81602, 19920101};

    std::ifstream file("../baselines/clickhouse/query_tpch_T1_range0.10_rerun_converted");
    std::ofstream outfile("query_tpch_T1_range0.10_trinity");

    for (int i = 0; i < 200; i ++) {

      std::vector<std::vector<int32_t>> found_points;
      std::vector<int32_t> start_range = min_values;
      std::vector<int32_t> end_range = max_values;

      std::string line;
      std::getline(file, line);
      std::stringstream ss(line);
      // Example: 0,-1,24,2,5,7,4,19943347,19950101
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
      start = GetTimestamp();
      client.range_search_trie(found_points, start_range, end_range);
      diff = GetTimestamp() - start;
      outfile << "Query " << i << " end to end latency (s): " << diff / 1000000 << ", found points count: " << found_points.size() << std::endl;
    }
}