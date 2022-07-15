#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "MDTrieShardClient.h"
// #include "TrinityBenchShared.h"
#include "TrinityNewBench.h"
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
const int client_num = 120;

int main(int argc, char *argv[]){

    if (argc != 2) {
      cerr << "./TrinityTPCHInsert [dataset part]" << endl;
    }
    int which_part = stoi(argv[1]);

    std::vector<std::string> server_ips = {"10.10.1.12", "10.10.1.13", "10.10.1.14", "10.10.1.15", "10.10.1.16"};

    total_points_count = 1000000000;
    auto client = MDTrieClient(server_ips, shard_num);

    if (!client.ping(2)){
        std::cerr << "Server setting wrong!" << std::endl;
        exit(-1);
    }

    /** 
        Insert all points
    */

    uint32_t throughput;
    throughput = total_client_insert(shard_num, client_num, server_ips, which_part);
    cout << which_part << ": Lookup Throughput (pt / seconds): " << throughput << endl;    
}