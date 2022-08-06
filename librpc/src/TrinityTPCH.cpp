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

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
const int DIMENSION = 9;
const int shard_num = 30;
const int client_num = 64;

int main(){

    std::vector<std::string> server_ips = {"10.254.254.229", "10.254.254.253"};
    const char *file_address = "/mntData2/tpch-dbgen/data_200/orders_lineitem_merged_by_chunk_indexed.csv";
    
    total_points_count = 1200018434;

    auto client = MDTrieClient(server_ips, shard_num);

    // for (unsigned int i = 0; i < server_ips.size(); ++i) {
    //   for (int j = 0; j < shard_num; j++){
    //     server_to_client.push_back({});
    //   }
    // }
    if (!client.ping(1)){
        std::cerr << "Server setting wrong!" << std::endl;
        exit(-1);
    }

    TimeStamp start, diff;

    /** 
        Insert all points
    */

    std::tuple<uint32_t, float> return_tuple;
    uint32_t throughput;

    // The range here is important! Must not overshoot the bit width set in the Trie!
    // std::vector<int32_t> max_values = {255, 2147483646, 255, 255, 2147483646, 2147483646, 2147483646, 2147483646, 2147483646};
    // std::vector<int32_t> min_values = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    // Narrower Range
    std::vector<int32_t> max_values = {50, 10495000, 10, 8, 19981201, 19981031, 19981231, 58211292, 19980802};
    std::vector<int32_t> min_values = {1, 90006, 0, 0, 19920102, 19920131, 19920103, 81347, 19920101};

    // vector<vector <int32_t>> *data_vector = check_data_vector_tpch();
    // return 0;

    start = GetTimestamp();
    throughput = total_client_insert(file_address, shard_num, client_num, server_ips);
    diff = GetTimestamp() - start;

    cout << "Insertion Throughput (pt / seconds): " << throughput << endl;
    cout << "End-to-end Latency (us): " << diff << endl;
    cout << "Storage: " << client.get_size() << endl;

    std::vector<int32_t> found_points;
    std::vector<int32_t> start_range(DIMENSION, 0);
    std::vector<int32_t> end_range(DIMENSION, 0);

    /** 
        Query 1
    */

    // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
    for (dimension_t i = 0; i < DIMENSION; i++){
        start_range[i] = min_values[i];
        end_range[i] = max_values[i];

        if (i == 4) 
        {
            start_range[i] = 19940101;
            end_range[i] = 19950101;  
        }
        if (i == 2){ 
            start_range[i] = 5;
            end_range[i] = 7;  
        }
        if (i == 0)
            end_range[i] = 24;
    }

    found_points.clear();
    start = GetTimestamp();
    client.range_search_trie(found_points, start_range, end_range);
    diff = GetTimestamp() - start;

    std::cout << "Query 1 end to end latency: " << diff << std::endl;  
    std::cout << "Found points count: " << found_points.size() << std::endl;
    // Correct Size: 23892962 (matched by Clickhouse)

    /**   
        Point Lookup given primary key
    */

    start = GetTimestamp();
    throughput = total_client_lookup(shard_num, client_num, server_ips);

    diff = GetTimestamp() - start;
    cout << "Primary Key Lookup Throughput (pt / seconds): " << throughput << endl;
    
    return 0;
}

/**   
    Insertion Throughput (pt / seconds): 958917
    End-to-end Latency (us): 1312271436
    Storage: 50205921310
    Query 1 end to end latency: 10609785
    Found points count: 23892962
    Primary Key Lookup Throughput (pt / seconds): 759078
*/
