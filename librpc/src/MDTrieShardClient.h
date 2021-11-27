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
      shard_vector_.push_back(launch_port(START_PORT_NUMBER + i, "172.29.249.30"));
    }
  }

  MDTrieClient(std::vector<std::string> server_ips){

    shard_vector_.reserve(server_ips.size());

    for (unsigned int i = 0; i < server_ips.size(); ++i) {
      shard_vector_.push_back(launch_port(9090, server_ips[i]));
    }
  }

  MDTrieClient(std::vector<std::string> server_ips, int shard_count){

    shard_vector_.reserve(server_ips.size());

    for (unsigned int i = 0; i < server_ips.size(); ++i) {
      for (int j = 0; j < shard_count; j++){
        shard_vector_.push_back(launch_port(9090 + j, server_ips[i]));
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

  // std::vector<int32_t> range_search_trie_each_rec(int index){
    
  //   std::vector<int32_t> return_vect_tmp;
  //   shard_vector_[index].recv_range_search_trie(return_vect_tmp);
  //   return return_vect_tmp;
  // }

  // void range_search_trie_rec(std::vector<int32_t> & return_vect){

  //   int client_count = shard_vector_.size();
  //   std::vector<std::future<std::vector<int32_t>>> threads; 

  //   for (uint8_t i = 0; i < client_count; i++){
  //     threads.push_back(std::async(range_search_trie_each_rec, i));
  //   }

  //   TimeStamp start = GetTimestamp();
  //   for (int i = 0; i < client_count; i++){

  //     std::vector<int32_t> return_vect_tmp = threads[i].get();
  //     return_vect.insert(return_vect.end(), return_vect_tmp.begin(), return_vect_tmp.end());
  //   }
  //   cout << "Latency: " << GetTimestamp() - start << " for: " << return_vect.size() << endl;

  // }

  void range_search_trie_rec(std::vector<int32_t> & return_vect){

    int client_count = shard_vector_.size();
    
    TimeStamp diff_recv = 0;
    TimeStamp diff_vect = 0;
    TimeStamp start = 0;

    for (uint8_t i = 0; i < client_count; i++){
      std::vector<int32_t> return_vect_tmp;

      start = GetTimestamp();
      shard_vector_[i].recv_range_search_trie(return_vect_tmp);
      diff_recv += GetTimestamp() - start;

      start = GetTimestamp();
      return_vect.insert(return_vect.end(), return_vect_tmp.begin(), return_vect_tmp.end());
      diff_vect += GetTimestamp() - start;
    }    

    cout << "Time taken for recv: " << diff_recv << " time taken for vect: " << diff_vect << endl;
    cout << "Throughput: " << ((float) return_vect.size() / diff_recv) * 1000000 << endl;
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