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
const level_t trie_depth = 10;
const preorder_t max_tree_node = 512;
const dimension_t DIMENSION = 4;

class MDTrieHandler : public MDTrieShardIf {
public:

  MDTrieHandler(){
    
    mdtrie_ = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
    std::vector<level_t> bit_widths = {8, 32, 32, 32}; // 4 Dimensions
    std::vector<level_t> start_bits = {0, 0, 0, 0}; // 4 Dimensions;
 
    total_points_count = 155846019;

    bitmap::CompactPtrVector tmp_ptr_vect(total_points_count);
    p_key_to_treeblock_compact = &tmp_ptr_vect;
    create_level_to_num_children(bit_widths, start_bits, 32);

  };

  void clear_trie(){

    delete mdtrie_; // TODO
    mdtrie_ = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
  }

  bool ping(const int32_t dataset_idx) { 

    cout << "ping(): [" << dataset_idx << "]" << endl; 

    if (dataset_idx == 0) // FS
    {
      if (DIMENSION != 7 || total_points_count != 14583357 || DIMENSION != dimension_to_num_bits.size() || DIMENSION != start_dimension_bits.size())
        return false;
    }
    else if (dataset_idx == 1) // OSM
    {
      if (DIMENSION != 4 || total_points_count != 155846019 || DIMENSION != dimension_to_num_bits.size() || DIMENSION != start_dimension_bits.size())
        return false;
    }
    else if (dataset_idx == 2) // TPC-H
    {
      if (DIMENSION != 9 || total_points_count != 300005812 || DIMENSION != dimension_to_num_bits.size() || DIMENSION != start_dimension_bits.size())
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

  int32_t insert(const std::vector<int32_t> & point, int32_t primary_key){

    data_point<DIMENSION> leaf_point;

    for (uint8_t i = 0; i < DIMENSION; i++)
      leaf_point.set_coordinate(i, point[i]);
    
    mdtrie_->insert_trie(&leaf_point, primary_key);
    
    return primary_key;
  }

  void range_search(std::vector<int32_t> & _return, const std::vector<int32_t> & start_range, const std::vector<int32_t> & end_range){
    
    data_point<DIMENSION> start_range_point;
    for (uint8_t i = 0; i < DIMENSION; i++)
      start_range_point.set_coordinate(i, start_range[i]);    

    data_point<DIMENSION> end_range_point;
    for (uint8_t i = 0; i < DIMENSION; i++)
      end_range_point.set_coordinate(i, end_range[i]);     

    mdtrie_->range_search_trie(&start_range_point, &end_range_point, mdtrie_->root(), 0, _return);
  }

  void primary_key_lookup(std::vector<int32_t> & _return, const int32_t primary_key){

    _return.reserve(DIMENSION);

    std::vector<morton_t> node_path_from_primary(max_depth + 1);
    tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *) (p_key_to_treeblock_compact->At(primary_key));

    morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(primary_key, node_path_from_primary);
    node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

    auto returned_coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);  
    
    for (uint8_t i = 0; i < DIMENSION; i++){
      _return.emplace_back(returned_coordinates->get_coordinate(i));
    }      
  }

  int32_t get_size(){
    return mdtrie_->size();
  }

protected:

  md_trie<DIMENSION> *mdtrie_; 
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