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
// #include "benchmark.hpp"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

level_t max_depth = 32;
level_t trie_depth = 6;
const preorder_t max_tree_node = 512;
// const dimension_t DIMENSION = 9;
const int shard_num = 20;

#define SERVER_LATENCY

#define TPCH_DIMENSION 9
#define GITHUB_DIMENSION 10
#define NYC_DIMENSION 15

enum {
    TPCH = 1,
    GITHUB = 2,
    NYC = 3,
};

template<dimension_t DIMENSION>
class MDTrieHandler : public MDTrieShardIf {
public:

  MDTrieHandler(int ip_address){
    mdtrie_ = nullptr;
    p_key_to_treeblock_compact_ = nullptr;
    outfile_.open(std::to_string(ip_address) + ".log", ios::out);
  };

  void clear_trie(){
    exit(0);
  }

  bool ping(const int32_t dataset_idx) { 

    if (mdtrie_ != nullptr && p_key_to_treeblock_compact_ != nullptr)
      return true;

    if (dataset_idx == GITHUB) // Github
    { 
      std::vector<level_t> bit_widths = {24, 24, 24, 24, 24, 24, 24, 16, 24, 24}; // 10 Dimensions;
      std::vector<level_t> start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 10 Dimensions;

      trie_depth = 6;
      max_depth = 24;
      no_dynamic_sizing = true;
      total_points_count = 828056295 / (shard_num * 5) + 1; 

      if (DIMENSION != 10) {
        std::cerr << "wrong config!" << DIMENSION << ", " << dataset_idx << std::endl;
        exit(-1);
      }
      p_key_to_treeblock_compact_ = new bitmap::CompactPtrVector(total_points_count);
      create_level_to_num_children(bit_widths, start_bits, 24);
      mdtrie_ = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);

      std::cout << "Github experiment started" << std::endl; 
    }

    else if (dataset_idx == TPCH) // TPC-H
    {
      std::vector<level_t> bit_widths = {8, 32, 16, 24, 32, 32, 32, 32, 32}; // 9 Dimensions;
      std::vector<level_t> start_bits = {0, 0, 8, 16, 0, 0, 0, 0, 0}; // 9 Dimensions;
      
      bit_widths = {8, 32, 16, 24, 20, 20, 20, 32, 20};
      trie_depth = 6;
      max_depth = 32;
      no_dynamic_sizing = true;
      total_points_count = 1000000000 / (shard_num * 5) + 1; 

      if (DIMENSION != 9) {
        std::cerr << "wrong config!" << DIMENSION << ", " << dataset_idx << std::endl;
        exit(-1);
      }
      p_key_to_treeblock_compact_ = new bitmap::CompactPtrVector(total_points_count);
      create_level_to_num_children(bit_widths, start_bits, max_depth);
      mdtrie_ = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
      std::cout << "Tpch experiment started" << std::endl; 
    }

    else if (dataset_idx == NYC) // NYC Taxi
    {
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
        std::cerr << "wrong config!" << DIMENSION << ", " << dataset_idx << std::endl;
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

  int32_t insert_for_latency(const std::vector<int32_t> & point, const int32_t primary_key){

    #ifdef SERVER_LATENCY
    TimeStamp start = GetTimestamp();
    #endif

    data_point<DIMENSION> leaf_point;

    for (uint8_t i = 0; i <= DIMENSION ; i++) {
      leaf_point.set_coordinate(i, point[i]);
    }

    mdtrie_->insert_trie(&leaf_point, inserted_points_, p_key_to_treeblock_compact_);

    inserted_points_ ++;

    for (const auto &coordinate : point) outfile_ << coordinate << ",";
    outfile_ << std::endl;

    #ifdef SERVER_LATENCY
    insert_latency_list_.push_back(GetTimestamp() - start);
    #endif
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
    #ifdef SERVER_LATENCY
    TimeStamp start = GetTimestamp();
    #endif

    std::vector<morton_t> node_path_from_primary(max_depth + 1);
    tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *) (p_key_to_treeblock_compact_->At(primary_key % inserted_points_));


    morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(primary_key % inserted_points_, node_path_from_primary);
    node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

    _return =  t_ptr->node_path_to_coordinates_vect(node_path_from_primary, DIMENSION); 
    #ifdef SERVER_LATENCY
    lookup_latency_list_.push_back(GetTimestamp() - start);
    #endif 
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

  void get_insert_latency(std::vector<int32_t> &_return){
    _return = insert_latency_list_;
  }

  void get_lookup_latency(std::vector<int32_t> &_return){
    _return = lookup_latency_list_;
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
  #ifdef SERVER_LATENCY
  std::vector<int32_t> insert_latency_list_;
  std::vector<int32_t> lookup_latency_list_;
  #endif
};

template<dimension_t DIMENSION>
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

        auto handler = std::make_shared<MDTrieHandler<DIMENSION>>(port_num);
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

    std::string ip_addr;
    int num_shards;
    int dataset_idx;
    char arg;

    while ((arg = getopt (argc, argv, "i:s:d:")) != -1) {
        switch (arg) {
            case 'i':
                ip_addr = std::string(optarg);
                break;
            case 's':
                num_shards = stoi(optarg);
                break;
            case 'd':
                dataset_idx = stoi(optarg);
                break;
            default:
                abort ();
        }
    }

  if (dataset_idx == TPCH){
      MDTrieServerCoordinator<TPCH_DIMENSION>(ip_addr, 9090, num_shards);
  }
  
  if (dataset_idx == GITHUB){
      MDTrieServerCoordinator<GITHUB_DIMENSION>(ip_addr, 9090, num_shards);
  }

  if (dataset_idx == NYC){
      MDTrieServerCoordinator<NYC_DIMENSION>(ip_addr, 9090, num_shards);
  }

  return 0;
}