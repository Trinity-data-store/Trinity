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

    std::vector<std::string> server_ips = {"10.10.1.3", "10.10.1.4", "10.10.1.5", "10.10.1.6", "10.10.1.7"};

    total_points_count = 828056295;
    auto client = MDTrieClient(server_ips, shard_num);

    /** 
        Insert all points
    */

    TimeStamp start, diff;
    uint32_t throughput;

    
    /** 
        Measure lookup throughput
    */

    start = GetTimestamp();
    throughput = total_client_lookup(shard_num, client_num, server_ips, which_part);
    diff = GetTimestamp() - start;

    cout << "Insertion Throughput (pt / seconds): " << throughput << endl;
    cout << "End-to-end Latency (s): " << diff / 1000000 << endl;
}