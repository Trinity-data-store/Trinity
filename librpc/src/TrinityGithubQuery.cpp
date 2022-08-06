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
const int DIMENSION = 10;
const int shard_num = 20;

int main(int argc, char *argv[]){

    if (argc < 2) {
      cerr << "./TrinityTPCHMacro [dataset part] [Infile] [Outfile]" << endl;
    }
    std::vector<std::string> server_ips = {"10.10.1.12", "10.10.1.13", "10.10.1.14", "10.10.1.15", "10.10.1.16"};

    total_points_count = 828056295;
    auto client = MDTrieClient(server_ips, shard_num);

    if (!client.ping(2)){
        std::cerr << "Server setting wrong!" << std::endl;
        exit(-1);
    }
    /** 
        Insert all points
    */
    
    TimeStamp start, diff;

    /** 
        Range Search
    */

    // [events_count, authors_count, forks, stars, issues, pushes, pulls, downloads, adds, dels, add_del_ratio, start_date, end_date]
    // MIN: [1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20110211, 20110211]
    // MAX: [7451541, 737170, 262926, 354850, 379379, 3097263, 703341, 8745, 3176317839, 1006504276, 117942850, 20201206, 20201206]

    // std::vector<int32_t> max_values = {7451541, 737170, 262926, 354850, 379379, 3097263, 703341, 8745, 3176317839, 1006504276, 117942850, 20201206, 20201206};
    // std::vector<int32_t> max_values = {7451541, 737170, 262926, 354850, 379379, 3097263, 703341, 8745, 2147483647, 1006504276, 117942850, 20201206, 20201206};
    std::vector<int32_t> max_values = {7451541, 737170, 262926, 354850, 379379, 3097263, 703341, 8745, 20201206, 20201206};

    // std::vector<int32_t> min_values = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20110211, 20110211};
    std::vector<int32_t> min_values = {1, 1, 0, 0, 0, 0, 0, 0, 20110211, 20110211};

    // char *infile_address = (char *)"/proj/trinity-PG0/Trinity/queries/github/github_query_new_converted";
    // char *infile_address = (char *)"/proj/trinity-PG0/Trinity/queries/github/github_query_new_timestamps_converted";
    // char *outfile_address = (char *)"query_github_trinity_timestamps";
    char *infile_address = (char *)"/proj/trinity-PG0/Trinity/queries/github/github_query_new_converted";
    char *outfile_address = (char *)"query_github_trinity_timestamps";
    cout << argc << endl;
    
    // if (argc == 3) {
    //   infile_address = argv[1];
    //   outfile_address = argv[2];
    // }
    // else {
    //   return 0;
    // }
    
    cout << infile_address << endl;
    cout << outfile_address << endl;
    std::ifstream file(infile_address);
    std::ofstream outfile(outfile_address, std::ios_base::app);

    for (int i = 0; i < 5; i ++) {

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

        cout << start_range_str << " " << end_range_str << endl;

        int index = std::stoul(index_str);
        if (index > 10)
          index -= 3;

        if (start_range_str != "-1") {
          start_range[static_cast<int32_t>(index)] = static_cast<int32_t>(std::stoul(start_range_str));
        }
        if (end_range_str != "-1") {
          end_range[static_cast<int32_t>(index)] = static_cast<int32_t>(std::stoul(end_range_str));
        }
      }

      for (dimension_t i = 0; i < DIMENSION; i++){
          if (i >= 8 || i == 9) {
              start_range[i] -= 20110000;
              end_range[i] -= 20110000;
          }
      }
    
      cout << "Query " << i << " started" << endl;
      start = GetTimestamp();

      client.range_search_trie(found_points, start_range, end_range);
      diff = GetTimestamp() - start;
      std::cout << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / DIMENSION << std::endl;
      // outfile << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / DIMENSION << std::endl;
      found_points.clear();
    }

}