#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "MDTrieShardClient.h"
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
const int client_num = 100;

int main(int argc, char *argv[]){

    if (argc < 2) {
      cerr << "./TrinityTPCHMacro [dataset part] [Infile] [Outfile]" << endl;
    }
    int which_part = stoi(argv[1]);

    std::vector<std::string> server_ips = {"10.10.1.12", "10.10.1.13", "10.10.1.14", "10.10.1.15", "10.10.1.16"};

    total_points_count = 1000000000;
    auto client = MDTrieClient(server_ips, shard_num);

    if (!client.ping(1)){
        std::cerr << "Server setting wrong!" << std::endl;
        exit(-1);
    }

    current_dataset_idx = 1;

    /** 
        Insert all points
    */

    uint32_t throughput;
    /** 
        Measure lookup throughput
    */

    throughput = total_client_lookup(shard_num, client_num, server_ips, which_part);

    cout << "Lookup Throughput (pt / seconds): " << throughput << endl;
}