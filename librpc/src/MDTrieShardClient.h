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

const int NUM_SERVERS = 72;
const int START_PORT_NUMBER = 9090;

class MDTrieClient {

public:

  MDTrieClient(){

    shard_vector_.reserve(NUM_SERVERS);

    for (int i = 0; i < NUM_SERVERS; ++i) {
      shard_vector_.push_back(launch_port(START_PORT_NUMBER + i));
    }
  }

  static MDTrieShardClient connect(const std::string &host, int port) {

    std::shared_ptr<TTransport> socket(new TSocket(host, port));
    std::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    MDTrieShardClient client(protocol);

    transport->open();
    return client;
  }

  static MDTrieShardClient launch_port(int port_num) {
    
    return connect("172.29.249.44", port_num);
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

  void insert_send(vector<int32_t> point, int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_insert_trie(point, p_key);
  }

  void insert_rec(int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].recv_insert_trie();
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
    shard_vector_[shard_index].send_primary_key_lookup(p_key);
    shard_vector_[shard_index].recv_primary_key_lookup(return_vect);

  }

  void primary_key_lookup_send(const int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_primary_key_lookup(p_key);
  }


  void primary_key_lookup_rec(std::vector<int32_t> & return_vect, const int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
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

  void range_search_trie_send(const std::vector<int32_t> & start_range, const std::vector<int32_t> & end_range){

    int client_count = shard_vector_.size();

    for (uint8_t i = 0; i < client_count; i++){
      shard_vector_[i].send_range_search_trie(start_range, end_range);
    }     
  }

  void range_search_trie_rec(std::vector<int32_t> & return_vect){

    int client_count = shard_vector_.size();

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