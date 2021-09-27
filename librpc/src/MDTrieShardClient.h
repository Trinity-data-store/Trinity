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

#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "trie.h"
#include "MDTrieShard.h"
#include <future>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;


class MDTrieClient {

public:

  MDTrieClient(int port_num, int client_count) {

    std::vector<std::future<void>> futures;
    shard_vector_.reserve(client_count);

    for (int i = 0; i < client_count; ++i) {
      shard_vector_.push_back(launch_port(port_num + i));
    }
  }

  static MDTrieShardClient connect(const std::string &host, int port) {

    cout << "Connecting to " << host << ":" << port << endl;
    std::shared_ptr<TTransport> socket(new TSocket(host, port));
    std::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    MDTrieShardClient client(protocol);

    transport->open();
    return client;
  }


  static MDTrieShardClient launch_port(int port_num) {
    
    return connect("localhost", port_num);
  
  }

  void ping(){

    int client_count = shard_vector_.size();
    for (uint8_t i = 0; i < client_count; i++)
      shard_vector_[i].ping();
  }

  void insert(vector<int32_t> point, int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_insert_trie(point, p_key);
    shard_vector_[shard_index].recv_insert_trie();
  }

  bool check(vector<int32_t> point, int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_check(point);
    return shard_vector_[shard_index].recv_check();
  }

  void primary_key_lookup(std::vector<int32_t> & return_vect, const int32_t p_key){

    int shard_index = p_key % shard_vector_.size();

    shard_vector_[shard_index].send_primary_key_lookup(p_key);
    shard_vector_[shard_index].recv_primary_key_lookup(return_vect);

  }

  void range_search_trie(std::vector<int32_t> & return_vect, const std::vector<int32_t> & start_range, const std::vector<int32_t> & end_range){

    int client_count = shard_vector_.size();

    for (uint8_t i = 0; i < client_count; i++){
      shard_vector_[i].send_range_search_trie(start_range, end_range);
    }     

    for (uint8_t i = 0; i < client_count; i++){
      std::vector<int32_t> return_vect_tmp;
      shard_vector_[i].recv_range_search_trie(return_vect_tmp);

      TimeStamp start = GetTimestamp();
      return_vect.insert(return_vect.end(), return_vect_tmp.begin(), return_vect_tmp.end());
      thrift_vector_time += GetTimestamp() - start;
    }    

  }

  void get_time(){

    shard_vector_[0].get_time();
  }

  int32_t get_count(){

    int client_count = shard_vector_.size();

    int32_t count = 0;
    for (uint8_t i = 0; i < client_count; i++){
      count += shard_vector_[i].get_count();
    }        
    return count;
  }


private:
  std::vector<MDTrieShardClient> shard_vector_; 
};
