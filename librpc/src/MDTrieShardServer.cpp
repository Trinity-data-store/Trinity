/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

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

const int DIMENSION = 6; 
const symbol_t NUM_BRANCHES = pow(2, DIMENSION);
const level_t max_depth = 32;
const level_t trie_depth = 10;
const preorder_t max_tree_node = 512;

class MDTrieHandler : public MDTrieShardIf {
public:

  MDTrieHandler(){
    mdtrie_ = new md_trie<DIMENSION, NUM_BRANCHES>(max_depth, trie_depth, max_tree_node);
  };

  void ping() { cout << "ping()" << endl; }

  int32_t add(const int32_t n1, const int32_t n2) {
    cout << "add(" << n1 << ", " << n2 << ")" << endl;
    return n1 + n2;
  }

  int32_t insert_trie(const std::vector<int32_t> & point){

    auto *leaf_point = new data_point<DIMENSION>();

    for (uint8_t i = 0; i < DIMENSION; i++)
      leaf_point->set_coordinate(i, point[i]);
    
    mdtrie_->insert_trie(leaf_point, max_depth);
    // TODO: return primary key (insert_trie function should return primary key)
    return current_primary_key - 1;
  }

  bool check(const std::vector<int32_t> & point){

    auto *leaf_point = new data_point<DIMENSION>();

    for (uint8_t i = 0; i < DIMENSION; i++)
      leaf_point->set_coordinate(i, point[i]);

    return mdtrie_->check(leaf_point, max_depth);
  }

  void range_search_trie(std::vector<std::vector<int32_t> > & _return, const std::vector<int32_t> & start_range, const std::vector<int32_t> & end_range){

    auto *start_range_point = new data_point<DIMENSION>();

    for (uint8_t i = 0; i < DIMENSION; i++)
      start_range_point->set_coordinate(i, start_range[i]);    

    auto *end_range_point = new data_point<DIMENSION>();

    for (uint8_t i = 0; i < DIMENSION; i++)
      end_range_point->set_coordinate(i, end_range[i]);     

    auto *found_points = new point_array<DIMENSION>();

    mdtrie_->range_search_trie(start_range_point, end_range_point, mdtrie_->root(), 0, found_points);

    n_leaves_t n_found_points = found_points->size();
    cout << "Range Search found " << n_found_points << " points" << endl;  

    for (n_leaves_t i = 0; i < n_found_points; i++){

      auto current_point = found_points->at(i)->get();
    
      std::vector<int32_t> vector_point;

      for (uint8_t i = 0; i < DIMENSION; i++){
        vector_point.push_back(current_point[i]);
      }

      _return.push_back(vector_point);
    }

  }

  void primary_key_lookup(std::vector<int32_t> & _return, const int32_t primary_key){

    _return.reserve(DIMENSION);

    symbol_t *node_path_from_primary = (symbol_t *)malloc((max_depth + 1) * sizeof(symbol_t));
    tree_block<DIMENSION, NUM_BRANCHES> *t_ptr = (tree_block<DIMENSION, NUM_BRANCHES> *) (p_key_to_treeblock_compact.At(primary_key));

    symbol_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(primary_key, node_path_from_primary);
    node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

    auto returned_coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary);      
    for (uint8_t i = 0; i < DIMENSION; i++){
      _return[i] = returned_coordinates->get_coordinate(i);
    }      

  }
  

protected:

  md_trie<DIMENSION, NUM_BRANCHES> *mdtrie_; 

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

    MDTrieServerCoordinator(int port_num) {

      start_server(port_num);
    }

    MDTrieServerCoordinator(int port_num, int server_count) {

        std::vector<std::future<void>> futures;

        for(int i = 0; i < server_count; ++i) {
          futures.push_back(std::async(start_server, port_num + i));
        }

        for(auto &e : futures) {
          e.get();
        }
    }

    static void start_server(int port_num){

        TThreadedServer server(
        std::make_shared<MDTrieShardProcessorFactory>(std::make_shared<MDTrieCloneFactory>()),
        std::make_shared<TServerSocket>(port_num), //port
        std::make_shared<TBufferedTransportFactory>(),
        std::make_shared<TBinaryProtocolFactory>());
        
        cout << "Starting the server..." << endl;
        server.serve();
        cout << "Done." << endl;
    }

private:

};


int main(int argc, char *argv[]){

  if (argc == 2){
    MDTrieServerCoordinator(atoi(argv[1]));
    return 0;
  }

  MDTrieServerCoordinator(9090, 10);
  return 0;
  
}