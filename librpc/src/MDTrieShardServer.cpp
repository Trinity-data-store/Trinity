#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/transport/TNonblockingServerTransport.h>
#include <queue>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <future>
#include <fstream>
#include "MDTrieShard.h"
#include "trie.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

level_t max_depth = 32;
level_t trie_depth = 6;
const preorder_t max_tree_node = 512;
const dimension_t DIMENSION = 9;
const int shard_num = 20;

class MDTrieHandler : public MDTrieShardIf {
public:

  MDTrieHandler(int ip_address){
    mdtrie_ = nullptr;
    p_key_to_treeblock_compact_ = nullptr;
    outfile_.open(std::to_string(ip_address) + ".log", ios::out);
  };

  void clear_trie(){

    exit(0);
    delete mdtrie_; // TODO
    delete p_key_to_treeblock_compact_;
    mdtrie_ = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
    p_key_to_treeblock_compact_ = new bitmap::CompactPtrVector(total_points_count);
  }

  bool ping(const int32_t dataset_idx) { 

    if (mdtrie_ != nullptr && p_key_to_treeblock_compact_ != nullptr)
      return true;

    if (dataset_idx == 2) // Github
    {
      // [events_count, authors_count, forks, stars, issues, pushes, pulls, downloads, adds, dels, add_del_ratio, start_date, end_date]
      // MIN: [1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20110211, 20110211]
      // MAX: [7451541, 737170, 262926, 354850, 379379, 3097263, 703341, 8745, 3176317839, 1006504276, 117942850, 20201206, 20201206]
      // std::vector<level_t> bit_widths = {24, 24, 24, 24, 24, 24, 24, 16, 32, 32, 32, 24, 24}; // 13 Dimensions;
      // std::vector<level_t> start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 13 Dimensions;
      
      std::vector<level_t> bit_widths = {24, 24, 24, 24, 24, 24, 24, 16, 24, 24}; // 10 Dimensions;
      std::vector<level_t> start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 10 Dimensions;

      trie_depth = 6;
      max_depth = 24;
      no_dynamic_sizing = true;
      total_points_count = 828056295 / (shard_num * 5) + 1; 

      if (DIMENSION != 10) {
        std::cerr << "wrong config!\n" << std::endl;
        exit(-1);
      }
      p_key_to_treeblock_compact_ = new bitmap::CompactPtrVector(total_points_count);
      create_level_to_num_children(bit_widths, start_bits, 24);
      mdtrie_ = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);

      std::cout << "Github experiment started" << std::endl; 
    }

    else if (dataset_idx == 1) // TPC-H
    {
      std::vector<level_t> bit_widths = {8, 32, 16, 24, 32, 32, 32, 32, 32}; // 9 Dimensions;
      std::vector<level_t> start_bits = {0, 0, 8, 16, 0, 0, 0, 0, 0}; // 9 Dimensions;
      // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
      // {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 58063825, 19980802};
      
      bit_widths = {8, 32, 16, 24, 20, 20, 20, 32, 20};

      // bit_widths = {8, 24, 16, 24, 20, 20, 20, 26, 20};  // TODO! Tried actually increase storage footprint...
      // start_bits = {0, 0, 8, 16, 0, 0, 0, 0, 0};

      trie_depth = 6;
      max_depth = 32;
      no_dynamic_sizing = true;
      total_points_count = 1000000000 / (shard_num * 5) + 1; 

      if (DIMENSION != 9) {
        std::cerr << "wrong config!\n" << std::endl;
        exit(-1);
      }
      p_key_to_treeblock_compact_ = new bitmap::CompactPtrVector(total_points_count);
      create_level_to_num_children(bit_widths, start_bits, max_depth);
      mdtrie_ = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
      std::cout << "Tpch experiment started" << std::endl; 
    }

    else if (dataset_idx == 3) // NYC Taxi
    {
      
      // pickup_date, dropoff_date, pickup_longitude, pickup_latitude, dropoff_longitude, dropoff_latitude, passenger_count, trip_distance, fare_amount, extra, mta_tax, tip_amount, tolls_amount, improvement_surcharge, total_amount
      // MIN: [20090101, 19700101, 0, 0, ... 0 ]
      // MAX: [20160630, 20221220, 89.9, 89.8, 89.9, 89.8, 255, 198623000, 21474808, 1000, 1311.2,  3950588.8, 21474836, 137.6, 21474830]

      // queried: trip_distance, fare_amount, tip_amount, pickup_date, dropoff_date, passenger_count. pickup_longitude, pickup_latitude

      std::vector<level_t> bit_widths = {18, 20, 10, 10, 10 + 10, 10 + 10, 8, 13, 10, 10 + 10, 11 + 9, 10, 10 + 10, 8, 10 + 10}; // 15 Dimensions;
      std::vector<level_t> start_bits = {0, 0, 0, 0, 0 + 10, 0 + 10, 0, 0, 0, 0 + 10, 0 + 9, 0, 0 + 10, 0, 0 + 10}; // 15 Dimensions; 24.54 GB, int


      bit_widths = {18, 20, 10, 10, 10 + 18, 10 + 18, 8, 28, 25, 10 + 18, 11 + 17, 22, 25, 8 + 20, 25}; // 15 Dimensions;
      start_bits = {0, 0, 0, 0, 0 + 18, 0 + 18, 0, 0, 0, 0 + 18, 0 + 17, 0, 0, 0 + 20, 0}; // 15 Dimensions; 24.54 GB, int

      trie_depth = 6;
      max_depth = 28;

      no_dynamic_sizing = true;
      total_points_count = 675200000 / (shard_num * 5) + 1; 
      total_points_count = 675200000;

      if (DIMENSION != 15) {
        std::cerr << "wrong config!\n" << std::endl;
        exit(-1);
      }
      p_key_to_treeblock_compact_ = new bitmap::CompactPtrVector(total_points_count);
      create_level_to_num_children(bit_widths, start_bits, max_depth);
      mdtrie_ = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);

      std::cout << "NYC experiment started" << std::endl; 
    }
    else 
    {
      cerr << "not implemented" << endl;
      return false;
    }      
    return true;
  }

  bool check(const std::vector<int32_t> & point){

    data_point<DIMENSION> leaf_point;

    for (uint8_t i = 0; i < DIMENSION; i++)
      leaf_point.set_coordinate(i, point[i]);

    bool result = mdtrie_->check(&leaf_point);
    return result;
  }

  int32_t insert(const std::vector<int32_t> & point, const int32_t primary_key){

    data_point<DIMENSION> leaf_point;

    for (uint8_t i = 0; i <= DIMENSION ; i++) {
      leaf_point.set_coordinate(i, point[i]);
    }

    mdtrie_->insert_trie(&leaf_point, inserted_points_, p_key_to_treeblock_compact_);
    inserted_points_ ++;

    for (const auto &coordinate : point) outfile_ << coordinate << ",";
    outfile_ << std::endl;

    return inserted_points_;
  }

  void range_search(std::vector<int32_t> & _return, const std::vector<int32_t> & start_range, const std::vector<int32_t> & end_range){
    
    data_point<DIMENSION> start_range_point;
    for (uint8_t i = 0; i < DIMENSION; i++)
      start_range_point.set_coordinate(i, start_range[i]);   


    data_point<DIMENSION> end_range_point;
    for (uint8_t i = 0; i < DIMENSION; i++) {
      end_range_point.set_coordinate(i, end_range[i]);   
    }  

    mdtrie_->range_search_trie(&start_range_point, &end_range_point, mdtrie_->root(), 0, _return);
    return;
  }

  void primary_key_lookup(std::vector<int32_t> & _return, const int32_t primary_key){

    // std::cout << primary_key << "," << inserted_points_  << std::endl;
    std::vector<morton_t> node_path_from_primary(max_depth + 1);
    tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *) (p_key_to_treeblock_compact_->At(primary_key % inserted_points_));

    morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(primary_key % inserted_points_, node_path_from_primary);
    node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

    _return =  t_ptr->node_path_to_coordinates_vect(node_path_from_primary, DIMENSION);  
  }

  void primary_key_lookup_path(std::vector<int32_t> & _return, const int32_t primary_key){

    std::vector<morton_t> node_path_from_primary(max_depth + 1);
    tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *) (p_key_to_treeblock_compact_->At(primary_key));

    morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(primary_key, node_path_from_primary);
    node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

     _return.reserve(max_depth);

    for (uint8_t i = 0; i < max_depth; i++){
      _return.emplace_back(node_path_from_primary[i]);
    } 
  }

  void primary_key_lookup_binary(std::string & _return, const int32_t primary_key){

    std::vector<morton_t> node_path_from_primary(max_depth + 1);
    tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *) (p_key_to_treeblock_compact_->At(primary_key));

    morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(primary_key, node_path_from_primary);
    node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

    std::vector<int32_t> return_vect;
    return_vect =  t_ptr->node_path_to_coordinates_vect(node_path_from_primary, DIMENSION);  

    std::stringstream ss;


    for (uint8_t i = 0; i < DIMENSION; i++){
      ss << return_vect[i] << ",";
    } 
    _return = ss.str();
  }

  int32_t get_size(){
    return mdtrie_->size(p_key_to_treeblock_compact_) + sizeof(int32_t) * (DIMENSION + 1) * backup_points_.size();
  }

protected:

  md_trie<DIMENSION> *mdtrie_; 
  bitmap::CompactPtrVector *p_key_to_treeblock_compact_;
  uint64_t inserted_points_ = 0;
  std::vector<std::vector<int32_t>> backup_points_;
  // std::vector<int32_t> min_values_;
  ofstream outfile_;
  // std::queue<std::vector<int32_t> lookup_cache_;
  // std::vector<std::vector<int32_t> lookup_cache;
};

class MDTrieCloneFactory : virtual public MDTrieShardIfFactory {
 public:
  ~MDTrieCloneFactory() override = default;
  MDTrieShardIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo)
  {
    std::shared_ptr<TSocket> sock = std::dynamic_pointer_cast<TSocket>(connInfo.transport);
    cout << "Incoming connection\n";
    cout << "\tSocketInfo: "  << sock->getSocketInfo() << "\n";
    cout << "\tPeerHost: "    << sock->getPeerHost() << "\n";
    cout << "\tPeerAddress: " << sock->getPeerAddress() << "\n";
    cout << "\tPeerPort: "    << sock->getPeerPort() << "\n";
    return new MDTrieHandler(sock->getPeerPort());
  }
  void releaseHandler( MDTrieShardIf* handler) {
    delete handler;
  }
};

class MDTrieServerCoordinator {

public:

    MDTrieServerCoordinator(std::string ip_address, int port_num, int shard_count){
      std::vector<std::future<void>> futures;

      for(int i = 0; i < shard_count; ++i) {
        futures.push_back(std::async(start_server, port_num + i, ip_address));
      }

      for(auto &e : futures) {
        e.get();
      }
    }

    static void start_server(int port_num, std::string ip_address){

        auto handler = std::make_shared<MDTrieHandler>(port_num);
        auto processor = std::make_shared<MDTrieShardProcessor>(handler);
        auto socket = std::make_shared<TNonblockingServerSocket>(ip_address, port_num);
        auto server = std::make_shared<TNonblockingServer>(processor, socket);

        cout << "Starting the server..." << endl;
        server->serve();
        cout << "Done." << endl;
    }

private:

};

int main(int argc, char *argv[]){

  if (argc == 3){
      MDTrieServerCoordinator(argv[1], 9090, stoi(argv[2]));
  }
  else {
    std::cerr << "Wrong Input! ./MdTrieShardServer [IP Address] [Number of Shards]" << std::endl;
    exit(-1);
  }
  return 0;
}