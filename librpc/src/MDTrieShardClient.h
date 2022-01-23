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

  MDTrieClient(std::vector<std::string> server_ips, int shard_count){

    shard_vector_.reserve(server_ips.size() * shard_count);

    for (unsigned int i = 0; i < server_ips.size(); ++i) {
      for (int j = 0; j < shard_count; j++){
        shard_vector_.push_back(launch_port(9090 + j, server_ips[i]));
        client_to_server_.push_back({});
        server_to_client_.push_back({});
      }
    }
  }

  static MDTrieShardClient connect(const std::string &host, int port) {

    std::shared_ptr<TTransport> socket(new TSocket(host, port, std::make_shared<TConfiguration>(1000 * 1024 * 1024)));  // Set max message size
    std::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    MDTrieShardClient client(protocol);

    transport->open();
    return client;
  }

  static MDTrieShardClient launch_port(int port_num, std::string ip_address) {
    
    return connect(ip_address, port_num);
  }

  void clear_trie(){

    int client_count = shard_vector_.size();
    for (uint8_t i = 0; i < client_count; i++)
      shard_vector_[i].clear_trie();
  }

  bool ping(int32_t dataset_idx){

    int client_count = shard_vector_.size();
    for (uint8_t i = 0; i < client_count; i++)
      if (!shard_vector_[i].ping(dataset_idx)){
        return false;
      }
    return true;
  }

  void insert(vector<int32_t> point, int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_insert(point, p_key);
    int32_t returned_key = shard_vector_[shard_index].recv_insert();
    client_to_server_[shard_index][p_key] = returned_key;
    server_to_client_[shard_index][returned_key] = p_key;
  }

  void insert_send(vector<int32_t> point, int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_insert(point, p_key);
  }

  void insert_rec(int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    int32_t returned_key = shard_vector_[shard_index].recv_insert();
    client_to_server_[shard_index][p_key] = returned_key;
    server_to_client_[shard_index][returned_key] = p_key;   
  }

  bool check(vector<int32_t> point, int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_check(point);
    return shard_vector_[shard_index].recv_check();
  }

  void check_send(vector<int32_t> point, int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_check(point);
  }  

  bool check_rec(int32_t p_key){
    
    int shard_index = p_key % shard_vector_.size();
    return shard_vector_[shard_index].recv_check();
  }

  void primary_key_lookup(std::vector<int32_t> & return_vect, const int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_primary_key_lookup(client_to_server_[shard_index][p_key]);
    shard_vector_[shard_index].recv_primary_key_lookup(return_vect);

  }

  void primary_key_lookup_send(const int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_primary_key_lookup(client_to_server_[shard_index][p_key]);
  }


  void primary_key_lookup_rec(std::vector<int32_t> & return_vect, const int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].recv_primary_key_lookup(return_vect);
  }

  void range_search_trie(std::vector<int32_t> & return_vect, const std::vector<int32_t> & start_range, const std::vector<int32_t> & end_range){

    int client_count = shard_vector_.size();
    
    for (uint8_t i = 0; i < client_count; i++){
      shard_vector_[i].send_range_search(start_range, end_range);
    }     

    for (uint8_t i = 0; i < client_count; i++){
      std::vector<int32_t> return_vect_tmp;
      shard_vector_[i].recv_range_search(return_vect_tmp);
      // raise(SIGINT);
      // return_vect.insert(return_vect.end(), return_vect_tmp.begin(), return_vect_tmp.end());
      for (unsigned int j = 0; j < return_vect_tmp.size(); j++){
        return_vect.push_back(server_to_client_[i][return_vect_tmp[j]]);
      }
    }    
  }

  void range_search_trie_send(const std::vector<int32_t> & start_range, const std::vector<int32_t> & end_range){

    int client_count = shard_vector_.size();

    for (uint8_t i = 0; i < client_count; i++){
      shard_vector_[i].send_range_search(start_range, end_range);
    }     
  }

  void range_search_trie_rec(std::vector<int32_t> & return_vect){

    int client_count = shard_vector_.size();
    
    for (uint8_t i = 0; i < client_count; i++){
      std::vector<int32_t> return_vect_tmp;
      shard_vector_[i].recv_range_search(return_vect_tmp);
      // return_vect.insert(return_vect.end(), return_vect_tmp.begin(), return_vect_tmp.end());
      for (unsigned int j = 0; j < return_vect_tmp.size(); j++){
        return_vect.push_back(server_to_client_[i][return_vect_tmp[j]]);
      }    
    }    
  }

  int64_t get_size(){

    int client_count = shard_vector_.size();
    int64_t count = 0;
    for (int i = 0; i < client_count; i++){
      int64_t temp = shard_vector_[i].get_size();
      count += temp;
    }        
    return count;
  }

private:
  std::vector<MDTrieShardClient> shard_vector_; 
  std::vector<std::map<int32_t, int32_t>> client_to_server_;
  std::vector<std::map<int32_t, int32_t>> server_to_client_;

};