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

/*
std::vector<int32_t> *node_path_to_coordinates(std::vector<int32_t> &node_path, dimension_t dimension) const{
    
    // Will be free-ed in the benchmark code

    for (level_t i = 0; i < max_depth_; i++){
        morton_t current_symbol = node_path[i];
        morton_t current_symbol_pos = level_to_num_children[i] - 1;

        for (dimension_t j = 0; j < dimension; j++){

            if (dimension_to_num_bits[j] <= i || i < start_dimension_bits[j])
                continue;         

            level_t current_bit = GETBIT(current_symbol, current_symbol_pos);
            current_symbol_pos --;

            point_t coordinate = coordinates->get_coordinate(j);
            coordinate = (coordinate << 1) + current_bit;
            coordinates->set_coordinate(j, coordinate);
        }
    }
    return coordinates;
}
*/

class MDTrieClient {

public:

  MDTrieClient(std::vector<std::string> server_ips, int shard_count){

    shard_vector_.reserve(server_ips.size() * shard_count);
    shard_queried_cnt_.reserve(server_ips.size() * shard_count);
    for (unsigned int i = 0; i < server_ips.size(); ++i) {
      for (int j = 0; j < shard_count; j++){
        shard_vector_.push_back(launch_port(9090 + j, server_ips[i]));
        shard_queried_cnt_.push_back(0);
      }
    }
  }

  static MDTrieShardClient connect(const std::string &host, int port) {

    std::shared_ptr<TTransport> socket(new TSocket(host, port));  // Set max message size
    std::shared_ptr<TTransport> transport(new TFramedTransport(socket, std::make_shared<TConfiguration>(1000 * 1024 * 1024, 1000 * 1024 * 1024)));
    // cout << "getMaxFrameSize: " << transport.getMaxFrameSize() << endl;
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
    for (uint16_t i = 0; i < client_count; i++)
      shard_vector_[i].send_clear_trie();
  }

  bool ping(int32_t dataset_idx){

    int client_count = shard_vector_.size();
    for (uint16_t i = 0; i < client_count; i++) {
      if (!shard_vector_[i].ping(dataset_idx)){
        return false;
      }
    }
    return true;
  }


  int32_t insert_for_latency(vector<int32_t> point, int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_insert_for_latency(point, p_key);
    return shard_vector_[shard_index].recv_insert_for_latency();
  }


  int32_t insert(vector<int32_t> point, int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_insert(point, p_key);
    return shard_vector_[shard_index].recv_insert();
  }

  void insert_send(vector<int32_t> point, int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_insert(point, p_key);
  }

  int32_t insert_rec(int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    return shard_vector_[shard_index].recv_insert();
  }

  bool check(vector<int32_t> point, int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_check(point);
    return shard_vector_[shard_index].recv_check();
  }


  void get_insert_latency(std::vector<int32_t> & return_vect){
    int client_count = shard_vector_.size();
    for (uint16_t i = 0; i < client_count; i++){
      std::vector<int32_t> return_vect_tmp;
      shard_vector_[i].get_insert_latency(return_vect_tmp);
      return_vect.insert(return_vect.end(), return_vect_tmp.begin(), return_vect_tmp.end());
    }  
  }

  void get_lookup_latency(std::vector<int32_t> & return_vect){
    int client_count = shard_vector_.size();
    for (uint16_t i = 0; i < client_count; i++){
      std::vector<int32_t> return_vect_tmp;
      shard_vector_[i].get_lookup_latency(return_vect_tmp);
      return_vect.insert(return_vect.end(), return_vect_tmp.begin(), return_vect_tmp.end());
    }  
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
    shard_queried_cnt_[shard_index] ++;
    shard_vector_[shard_index].recv_primary_key_lookup(return_vect);

  }

  void primary_key_lookup_send(const int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].send_primary_key_lookup(p_key);
    shard_queried_cnt_[shard_index] ++;
  }

  void primary_key_lookup_rec(std::vector<int32_t> & return_vect, const int32_t p_key){

    int shard_index = p_key % shard_vector_.size();
    shard_vector_[shard_index].recv_primary_key_lookup(return_vect);
  }

  void primary_key_lookup_send_zipf(const int32_t p_key, const int32_t shard_key){

    int shard_index = shard_key % shard_vector_.size();
    shard_vector_[shard_index].send_primary_key_lookup(p_key);
    shard_queried_cnt_[shard_index] ++;
  }

  void primary_key_lookup_rec_zipf(std::vector<int32_t> & return_vect, const int32_t shard_key){

    int shard_index = shard_key % shard_vector_.size();
    shard_vector_[shard_index].recv_primary_key_lookup(return_vect);
  }


  void range_search_trie(std::vector<int32_t> & return_vect, const std::vector<int32_t> & start_range, const std::vector<int32_t> & end_range){

    int client_count = shard_vector_.size();
    
    for (uint16_t i = 0; i < client_count; i++){
      shard_vector_[i].send_range_search(start_range, end_range);
    }     

    for (uint16_t i = 0; i < client_count; i++){
      std::vector<int32_t> return_vect_tmp;
      shard_vector_[i].recv_range_search(return_vect_tmp);
      return_vect.insert(return_vect.end(), return_vect_tmp.begin(), return_vect_tmp.end());
    }    
  }

  void range_search_trie_send(const std::vector<int32_t> & start_range, const std::vector<int32_t> & end_range){

    int client_count = shard_vector_.size();

    for (uint16_t i = 0; i < client_count; i++){
      shard_vector_[i].send_range_search(start_range, end_range);
    }     
  }

  void range_search_trie_rec(std::vector<int32_t> & return_vect){

    int client_count = shard_vector_.size();
    
    for (uint16_t i = 0; i < client_count; i++){
      std::vector<int32_t> return_vect_tmp;
      shard_vector_[i].recv_range_search(return_vect_tmp);
      return_vect.insert(return_vect.end(), return_vect_tmp.begin(), return_vect_tmp.end());
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
  std::vector<int32_t> shard_queried_cnt_;
};