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

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <future>

#include "MDTrieShard.h"
#include "trie.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

const level_t max_depth = 32;
level_t trie_depth = 6;
const preorder_t max_tree_node = 512;
const dimension_t DIMENSION = 9;
unsigned int num_shards = 1;
const int shard_num = 40;

class MDTrieHandler : public MDTrieShardIf {
public:

  MDTrieHandler(){
    
    /** 
        FS
    */

    // std::vector<level_t> bit_widths = {32, 32, 32, 32, 24, 24, 32}; // 7 Dimensions    
    // std::vector<level_t> start_bits = {0, 0, 0, 0, 0, 0, 0}; // 7 Dimensions    
    // num_shards = 20 * 5;
    // trie_depth = 10;
    // no_dynamic_sizing = true;
    // total_points_count = 14583357 / num_shards + 1;    

    /** 
        OSM
    */

    // std::vector<level_t> bit_widths = {8, 32, 32, 32}; // 4 Dimensions
    // std::vector<level_t> start_bits = {0, 0, 0, 0}; // 4 Dimensions;
    // num_shards = 20 * 5;
    // trie_depth = 6;
    // no_dynamic_sizing = true;
    // total_points_count = 152806265 / num_shards + 1; 

    /** 
        TPCH
    */

    std::vector<level_t> bit_widths = {8, 32, 16, 24, 32, 32, 32, 32, 32}; // 9 Dimensions;
    std::vector<level_t> start_bits = {0, 0, 8, 16, 0, 0, 0, 0, 0}; // 9 Dimensions;
    num_shards = shard_num * 5;
    trie_depth = 6;
    no_dynamic_sizing = true;
    total_points_count = 3000028242 / num_shards + 1; 

    p_key_to_treeblock_compact_ = new bitmap::CompactPtrVector(total_points_count);
    create_level_to_num_children(bit_widths, start_bits, 32);
    mdtrie_ = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);

  };

  void clear_trie(){

    exit(0);
    delete mdtrie_; // TODO
    delete p_key_to_treeblock_compact_;
    mdtrie_ = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
    p_key_to_treeblock_compact_ = new bitmap::CompactPtrVector(total_points_count);
  }

  bool ping(const int32_t dataset_idx) { 

    cout << "ping(): [" << dataset_idx << "]" << endl; 

    if (dataset_idx == 0) // FS
    {
      if (DIMENSION != 7 || DIMENSION != dimension_to_num_bits.size() || DIMENSION != start_dimension_bits.size() || total_points_count != p_key_to_treeblock_compact_->get_num_elements())
        return false;
    }
    else if (dataset_idx == 1) // OSM
    {
      if (DIMENSION != 4 || DIMENSION != dimension_to_num_bits.size() || DIMENSION != start_dimension_bits.size() || total_points_count != p_key_to_treeblock_compact_->get_num_elements())
        return false;
    }
    else if (dataset_idx == 2) // TPC-H
    {
      if (DIMENSION != 9 || DIMENSION != dimension_to_num_bits.size() || DIMENSION != start_dimension_bits.size() || total_points_count != p_key_to_treeblock_compact_->get_num_elements())
        return false;
    }
    else 
      return false;
    return true;
  }

  int32_t add(const int32_t n1, const int32_t n2) {
    cout << "add(" << n1 << ", " << n2 << ")" << endl;
    return n1 + n2;
  }

  bool check(const std::vector<int32_t> & point){

    data_point<DIMENSION> leaf_point;

    for (uint8_t i = 0; i < DIMENSION; i++)
      leaf_point.set_coordinate(i, point[i]);

    bool result = mdtrie_->check(&leaf_point);
    return result;
  }

  int32_t insert(const std::vector<int32_t> & point){

    data_point<DIMENSION> leaf_point;

    for (uint8_t i = 0; i < DIMENSION; i++)
      leaf_point.set_coordinate(i, point[i]);

    mdtrie_->insert_trie(&leaf_point, inserted_points_, p_key_to_treeblock_compact_);
    inserted_points_ ++;
    return inserted_points_ - 1;
  }

  void range_search(std::vector<std::vector<int32_t>> & _return, const std::vector<int32_t> & start_range, const std::vector<int32_t> & end_range){
    
    data_point<DIMENSION> start_range_point;
    for (uint8_t i = 0; i < DIMENSION; i++)
      start_range_point.set_coordinate(i, start_range[i]);    

    data_point<DIMENSION> end_range_point;
    for (uint8_t i = 0; i < DIMENSION; i++)
      end_range_point.set_coordinate(i, end_range[i]);     
    
    std::vector<data_point<DIMENSION>> return_vect;
    mdtrie_->range_search_trie(&start_range_point, &end_range_point, mdtrie_->root(), 0, return_vect);

    // _return.reserve(return_vect.size());
    for (unsigned i = 0; i < return_vect.size(); i++) {
      std::vector<int32_t> return_pt(DIMENSION);

      for (unsigned j = 0; j < DIMENSION; j ++) {
        return_pt[j] = return_vect[i].get_coordinate(j);
      }
      // _return[i] = return_pt;
      _return.push_back(return_pt);
    }
  }

  void primary_key_lookup(std::vector<int32_t> & _return, const int32_t primary_key){

    _return.reserve(DIMENSION);

    std::vector<morton_t> node_path_from_primary(max_depth + 1);
    tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *) (p_key_to_treeblock_compact_->At(primary_key));

    morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(primary_key, node_path_from_primary);
    node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

    auto returned_coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);  
    
    for (uint8_t i = 0; i < DIMENSION; i++){
      _return.emplace_back(returned_coordinates->get_coordinate(i));
    }      
  }

  int32_t get_size(){
    return mdtrie_->size(p_key_to_treeblock_compact_);
  }

protected:

  md_trie<DIMENSION> *mdtrie_; 
  bitmap::CompactPtrVector *p_key_to_treeblock_compact_;
  uint64_t inserted_points_ = 0;
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
    return new MDTrieHandler;
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

        auto handler = std::make_shared<MDTrieHandler>();
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