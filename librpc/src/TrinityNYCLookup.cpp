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
const int client_num = 20;

int main(int argc, char *argv[]){

    if (argc != 2) {
        std::cerr << "wrong number of arguments\n";
        exit(-1);
    }
    int which_part = stoi(argv[1]);

    std::vector<std::string> server_ips = {"10.10.1.12", "10.10.1.13", "10.10.1.14", "10.10.1.15", "10.10.1.16"};

    total_points_count = 675200000;
    auto client = MDTrieClient(server_ips, shard_num);
    current_dataset_idx = 3;

    /** 
        Measure lookup throughput
    */

    uint32_t throughput;
    throughput = total_client_lookup(shard_num, client_num, server_ips, which_part);
    cout << "Lookup Throughput (pt / seconds): " << throughput << endl;
}