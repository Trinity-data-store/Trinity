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

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

const int shard_num = 20;
const int DIMENSION = 15;

int main(){

    std::vector<std::string> server_ips = {"10.10.1.12", "10.10.1.13", "10.10.1.14", "10.10.1.15", "10.10.1.16"};

    total_points_count = 675200000;
    auto client = MDTrieClient(server_ips, shard_num);

    /** 
        Insert all points
    */
    
    TimeStamp start, diff;

    /** 
        Range Search
    */

    // pickup_date, dropoff_date, pickup_longitude, pickup_latitude, dropoff_longitude, dropoff_latitude, passenger_count, trip_distance, fare_amount, extra, mta_tax, tip_amount, tolls_amount, improvement_surcharge, total_amount
    // MIN: [20090101, 19700101, 0, 0, ... 0 ]
    // MAX: [20160630, 20221220, 89.9, 89.8, 89.9, 89.8, 255, 198623000, 21474808, 1000, 1311.2,  3950588.8, 21474836, 137.6, 21474830]

    // std::vector<int32_t> max_values = {20160630, 20221220, 899, 898, 899, 898, 255, 1986230000, 214748080, 10000, 13112,  39505888, 214748360, 1376, 214748300};
    // std::vector<int32_t> min_values = {20090101, 19700101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    std::vector<int32_t> max_values = {20160630, 20221220, 899, 898, 899, 898, 255, 198623000, 21474808, 1000, 1312,  3950589, 21474836, 138, 21474830};
    std::vector<int32_t> min_values = {20090101, 19700101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


    char *infile_address = (char *)"/proj/trinity-PG0/Trinity/queries/nyc/nyc_query_new_converted";
    char *outfile_address = (char *)"/proj/trinity-PG0/Trinity/results/nyc_trinity_4T";
    
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

        while (ss.good()) {

            std::string index_str;
            std::getline(ss, index_str, ',');

            std::string start_range_str;
            std::getline(ss, start_range_str, ',');
            std::string end_range_str;
            std::getline(ss, end_range_str, ',');

            int index = std::stoul(index_str);

            if (start_range_str != "-1") {
                // if (index >= 2 && index != 6) {
                if (index >= 2 && index <= 5) {
                    float num_float = std::stof(start_range_str);
                    start_range[static_cast<int32_t>(index)] = static_cast<int32_t>(num_float * 10);                    
                }
                // else if (index >= 7){
                //     float num_float = std::stof(start_range_str);
                //     start_range[static_cast<int32_t>(index)] = static_cast<int32_t>(num_float);
                // }
                else 
                    start_range[static_cast<int32_t>(index)] = static_cast<int32_t>(std::stoul(start_range_str));
            }
            if (end_range_str != "-1") {
                if (index >= 2 && index <= 5) {
                    float num_float = std::stof(end_range_str);
                    end_range[static_cast<int32_t>(index)] = static_cast<int32_t>(num_float * 10);                    
                }
                // else if (index >= 7){
                //     float num_float = std::stof(end_range_str);
                //     end_range[static_cast<int32_t>(index)] = static_cast<int32_t>(num_float);
                // }
                else 
                    end_range[static_cast<int32_t>(index)] = static_cast<int32_t>(std::stoul(end_range_str));
            }
        }

        start_range[0] -= 20090000;
        start_range[1] -= 19700000;
        end_range[0] -= 20090000;
        end_range[1] -= 19700000;

        cout << "Query " << i << " started" << endl;
        start = GetTimestamp();
        client.range_search_trie(found_points, start_range, end_range);
        diff = GetTimestamp() - start;

        std::cout << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / DIMENSION << std::endl;
        outfile << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / DIMENSION << std::endl;
        found_points.clear();
    }

}