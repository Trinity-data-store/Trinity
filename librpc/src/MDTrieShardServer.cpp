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
#include "../../libmdtrie/bench/common.hpp"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

int num_shards = 20;
const int root_port = 9090;

template<dimension_t DIMENSION>
class MDTrieHandler : public MDTrieShardIf {
public:

  MDTrieHandler(int ip_address){
    mdtrie_ = nullptr;
    p_key_to_treeblock_compact_ = nullptr;
    outfile_.open(std::to_string(ip_address) + ".log", ios::out);
  };

  bool ping(const int32_t dataset_idx) { 

    if (mdtrie_ != nullptr && p_key_to_treeblock_compact_ != nullptr)
      return true;

    if (dataset_idx == GITHUB) // Github
    { 
      total_points_count = GITHUB_SIZE / (num_shards * 5) + 1; 
      use_github_setting(GITHUB_DIMENSION, total_points_count);
      mdtrie_ = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
      std::cout << "Github experiment started" << DIMENSION << "," << total_points_count <<  std::endl; 
    }

    else if (dataset_idx == TPCH) // TPC-H
    {
      total_points_count = TPCH_SIZE / (num_shards * 5) + 1; 
      use_github_setting(TPCH_DIMENSION, total_points_count);
      mdtrie_ = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
      std::cout << "Tpch experiment started: " << DIMENSION << "," << total_points_count <<  std::endl; 
    }

    else if (dataset_idx == NYC) // NYC Taxi
    {
      total_points_count = NYC_SIZE / (num_shards * 5) + 1; 
      use_nyc_setting(NYC_DIMENSION, total_points_count);
      mdtrie_ = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
      std::cout << "NYC experiment started" << DIMENSION << "," << total_points_count <<  std::endl; 
    }
    else 
    {
      std::cerr << "not implemented" << std::endl;
      return false;
    }      
    p_key_to_treeblock_compact_ = new bitmap::CompactPtrVector(total_points_count);
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
#ifdef USE_LINEAR_SCAN
    if (primary_key < 0)
    {
      std::cout << "Wrong!" << std::endl;
      exit(-1);
    }
    mdtrie_->insert_trie(&leaf_point, primary_key, p_key_to_treeblock_compact_);
#else
    mdtrie_->insert_trie(&leaf_point, inserted_points_, p_key_to_treeblock_compact_);
#endif
    inserted_points_ ++;
    for (const auto &coordinate : point) outfile_ << coordinate << ",";
    outfile_ << std::endl;
    return inserted_points_;
  }

  int32_t insert_for_latency(const std::vector<int32_t> & point, const int32_t primary_key){

    data_point<DIMENSION> leaf_point;
    for (uint8_t i = 0; i <= DIMENSION ; i++) {
      leaf_point.set_coordinate(i, point[i]);
    }
    
#ifdef USE_LINEAR_SCAN
    mdtrie_->insert_trie(&leaf_point, primary_key, p_key_to_treeblock_compact_);
#else
    mdtrie_->insert_trie(&leaf_point, inserted_points_, p_key_to_treeblock_compact_);
#endif
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

    std::vector<morton_t> node_path_from_primary(max_depth + 1);
#ifdef USE_LINEAR_SCAN
    tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *) (p_key_to_treeblock_compact_->At(primary_key));
    morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(primary_key, node_path_from_primary);
#else
    tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *) (p_key_to_treeblock_compact_->At(primary_key % inserted_points_));
    morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(primary_key % inserted_points_, node_path_from_primary);
#endif

    node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;
    _return =  t_ptr->node_path_to_coordinates_vect(node_path_from_primary, DIMENSION); 
  }

  int32_t get_size(){
    return mdtrie_->size(p_key_to_treeblock_compact_);
  }

protected:

  md_trie<DIMENSION> *mdtrie_; 
  bitmap::CompactPtrVector *p_key_to_treeblock_compact_;
  uint64_t inserted_points_ = 0;
  ofstream outfile_;
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
        server->serve();
    }
private:
};

int main(int argc, char *argv[]){

    std::string ip_addr;
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
      MDTrieServerCoordinator<TPCH_DIMENSION>(ip_addr, root_port, num_shards);
  }
  
  if (dataset_idx == GITHUB){
      MDTrieServerCoordinator<GITHUB_DIMENSION>(ip_addr, root_port, num_shards);
  }

  if (dataset_idx == NYC){
      MDTrieServerCoordinator<NYC_DIMENSION>(ip_addr, root_port, num_shards);
  }

  return 0;
}