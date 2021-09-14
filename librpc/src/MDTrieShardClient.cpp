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

#include "MDTrieShard.h"
#include <future>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

class MDTrieClientCoordinator {

public:
  MDTrieClientCoordinator(int port_num, int client_count) {

    std::vector<std::future<void>> futures;

    for(int i = 0; i < client_count; ++i) {
      futures.push_back(std::async(launch_port, port_num + i));
    }

    for(auto &e : futures) {
      e.get();
    }    
  }

  static MDTrieShardClient connect(const std::string &host, int port) {

    cout << "Connecting to " << host << ":" << port << endl;
    std::shared_ptr<TTransport> socket(new TSocket(host, port));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    MDTrieShardClient client(protocol);

    transport->open();
    return client;
  }


  static void launch_port(int port_num) {

    MDTrieShardClient client = connect("localhost", port_num);

    try {

      client.ping();

      cout << "ping()" << endl;

      cout << "1 + 1 = " << client.add(1, 1) << endl;

      vector<int32_t> point{ 10, 20, 30, 40, 50, 60 };
      
      cout << "primary key: " << client.insert_trie(point) << endl;

      if (!client.check(point)){
        cout << "not found!" << endl;
      } else {
        cout << "found!" << endl;
      }

      vector<int32_t> start_point{ 0, 0, 0, 0, 0, 0 };
      vector<int32_t> end_point{100, 100, 100, 100, 100, 100};
      vector<vector<int32_t>> range_search_return;

      client.range_search_trie(range_search_return, start_point, end_point);
      cout << "Range Search found " << range_search_return.size() << " points" << endl;

      vector<int32_t> lookup_return;
      client.primary_key_lookup(lookup_return, 0);
      
      for (uint8_t i = 0; i < lookup_return.size(); i++){
        if (point[i] != lookup_return[i]){
          cout << "point differ!" << endl;
        }
      }

      cout << "the same point" << endl;
    }
    
    catch (std::exception& exc){
      cerr << exc.what();
    }

  }

private:

};

int main(){

  MDTrieClientCoordinator(9090, 10);
  return 0;

}