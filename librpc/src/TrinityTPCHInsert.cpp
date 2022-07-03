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

    std::vector<std::string> server_ips = {"10.10.1.3", "10.10.1.4", "10.10.1.5", "10.10.1.6", "10.10.1.7"};

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
      diff = GetTimestamp() - start;

      cout << "Lookup Throughput (pt / seconds): " << throughput << endl;
      cout << "End-to-end Latency (s): " << diff / 1000000 << endl;
      cout << "Storage (MB): " << client.get_size() / 1000000 << endl;
    }
    else {
      cout << "which part = 0, exits" << endl;
    }
    
}