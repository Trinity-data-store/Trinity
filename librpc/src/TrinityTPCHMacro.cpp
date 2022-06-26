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

    if (argc < 2) {
      cerr << "./TrinityTPCHMacro [dataset part] [Infile] [Outfile]" << endl;
    }
    int which_part = stoi(argv[1]);

    // std::vector<std::string> server_ips = {"10.254.254.225", "10.254.254.213", "10.254.254.217", "10.254.254.205", "10.254.254.221"};
    std::vector<std::string> server_ips = {"10.10.1.5", "10.10.1.6", "10.10.1.7", "10.10.1.8", "10.10.1.9"};

    total_points_count = 1000000000;
    auto client = MDTrieClient(server_ips, shard_num);

    if (!client.ping(2)){
        std::cerr << "Server setting wrong!" << std::endl;
        exit(-1);
    }
    /** 
        Insert all points
    */

    TimeStamp start, diff;
    uint32_t throughput;

    if (which_part == 4) {
      cout << "Storage (MB): " << client.get_size() / 1000000 << endl;
      return 0;
    }
    if (which_part != 0) {
      start = GetTimestamp();
      throughput = total_client_insert_split_tpch(shard_num, client_num, server_ips, which_part);
      TimeStamp diff = GetTimestamp() - start;

      cout << "Insertion Throughput (pt / seconds): " << throughput << endl;
      cout << "End-to-end Latency (s): " << diff / 1000000 << endl;
      cout << "Storage (MB): " << client.get_size() / 1000000 << endl;
    }
    // One client does query for simplicity
    if (which_part == 2 || which_part == 3) {
      return 0;
    }
    
    /** 
        Range Search
    */

    // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
    std::vector<int32_t> max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 59591284, 19980802};
    std::vector<int32_t> min_values = {1, 90001, 0, 0, 19920102, 19920131, 19920103, 81602, 19920101};

    char *infile_address = (char *)"../baselines/clickhouse/query_tpch_rerun_converted";
    char *outfile_address = (char *)"query_tpch_trinity";
    cout << argc << endl;
    
    if (argc == 4) {
      infile_address = argv[2];
      outfile_address = argv[3];
    }
    else {
      return 0;
    }
    // client.clear_trie();
    
    cout << infile_address << endl;
    cout << outfile_address << endl;
    std::ifstream file(infile_address);
    std::ofstream outfile(outfile_address, std::ios_base::app);

    for (int i = 0; i < 1000; i ++) {

      std::vector<int32_t> found_points;
      std::vector<int32_t> start_range = min_values;
      std::vector<int32_t> end_range = max_values;

      std::string line;
      std::getline(file, line);

      std::stringstream ss(line);
      cout << line << endl;
      // Example: 0,-1,24,2,5,7,4,19943347,19950101
      while (ss.good()) {

        std::string index_str;
        std::getline(ss, index_str, ',');

        std::string start_range_str;
        std::getline(ss, start_range_str, ',');
        std::string end_range_str;
        std::getline(ss, end_range_str, ',');

        cout << start_range_str << " " << end_range_str << endl;
        if (start_range_str != "-1") {
          start_range[static_cast<int32_t>(std::stoul(index_str))] = static_cast<int32_t>(std::stoul(start_range_str));
        }
        if (end_range_str != "-1") {
          end_range[static_cast<int32_t>(std::stoul(index_str))] = static_cast<int32_t>(std::stoul(end_range_str));
        }
      }

      for (dimension_t i = 0; i < DIMENSION; i++){
          if (i >= 4 && i != 7) {
              start_range[i] -= 19000000;
              end_range[i] -= 19000000;
          }
      }
        

      cout << "Query " << i << " started" << endl;
      start = GetTimestamp();

      // if (i >= 99) {
      client.range_search_trie(found_points, start_range, end_range);
      diff = GetTimestamp() - start;
      // cout << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / DIMENSION << std::endl;
      outfile << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / DIMENSION << std::endl;
        // return 0;
      // }
      found_points.clear();
    }

}