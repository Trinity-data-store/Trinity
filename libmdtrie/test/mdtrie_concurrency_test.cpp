#include "catch.hpp"
#include "trie.h"
#include <mutex>
#include <thread>
#include <rand_utils.h>
#include <vector>

const int DIMENSION = 2;
const symbol_t NUM_BRANCHES = pow(2, DIMENSION);

level_t max_depth = 32;
level_t trie_depth = 10;
preorder_t max_tree_node = 1024;
n_leaves_t n_points = 500000;
uint8_t max_num_threads = 18;
const uint32_t read_number_count = 10000000;

void test_random_insert(md_trie *mdtrie){

    auto *leaf_point = new data_point();
    symbol_t range = pow(2, max_depth);
    auto *rand_generator = new utils::rand_utils();

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {

        for (dimension_t i = 0; i < DIMENSION; i++) {
            point_t coordinate = (point_t) rand_generator->rand_uint64(0, range - 1);
            leaf_point->set_coordinate(i, coordinate);
        }
        
        mdtrie->insert_trie(leaf_point, itr - 1);
    }
    return;    
}

void test_random_read(md_trie *mdtrie){

    auto *leaf_point = new data_point();
    symbol_t range = pow(2, max_depth);
    auto *rand_generator = new utils::rand_utils();

    for (n_leaves_t itr = 1; itr <= n_points; itr++) {

        for (dimension_t i = 0; i < DIMENSION; i++) {
            point_t coordinate = (point_t) rand_generator->rand_uint64(0, range - 1);
            leaf_point->set_coordinate(i, coordinate);
        }
        
        mdtrie->check(leaf_point);
    }
    return;    
}


std::vector<std::vector <int32_t>> *get_data_vector(){

/** 
    Get data from the OSM dataset stored in a vector
*/

  FILE *fp = fopen("../libmdtrie/bench/data/osm_combined_updated.csv", "r");
  char *line = nullptr;
  size_t len = 0;
  ssize_t read;

  if (fp == nullptr)
  {
      fprintf(stderr, "file not found\n");
      char cwd[PATH_MAX];
      if (getcwd(cwd, sizeof(cwd)) != nullptr) {
          printf("Current working dir: %s\n", cwd);
      } else {
          perror("getcwd() error");
      }
      exit(EXIT_FAILURE);
  }  

  n_leaves_t n_points = 0;
  auto data_vector = new std::vector<std::vector <int32_t>>;
  while ((read = getline(&line, &len, fp)) != -1)
  {

      std::vector<int32_t> point;
      char *token = strtok(line, ",");
      char *ptr;

      for (dimension_t i = 0; i < 8; i++){

          if (i == 1){
              token = strtok(nullptr, ",");
              token = strtok(nullptr, ",");
          }

          token = strtok(nullptr, ",");

          if (i < 8 - DIMENSION)
              continue;
          
          point.push_back(strtoul(token, &ptr, 10));
      }

      data_vector->push_back(point);
      n_points ++;
  }  
  return data_vector;
}

void test_osm_insert(md_trie *mdtrie, std::vector<std::vector <int32_t>> *data_vector, int total_partition, int partition_index)
{

  uint32_t start_pos = data_vector->size() / total_partition * partition_index;
  uint32_t end_pos = data_vector->size() / total_partition * (partition_index + 1) - 1;  

  if (end_pos >= data_vector->size())
    end_pos = data_vector->size() - 1;

  for (uint32_t i = start_pos; i <= end_pos; i++){
    // insertion_calls ++;
    std::vector <int32_t> point = (* data_vector)[i];
    auto *leaf_point = new data_point();

    for (uint8_t i = 0; i < DIMENSION; i++)
      leaf_point->set_coordinate(i, point[i]);
    
    mdtrie->insert_trie(leaf_point, i);
  }      

}

// TEST_CASE("Test OSM datasets", "[trie]") {

//     auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);
//     create_level_to_num_children(std::vector<level_t>(DIMENSION, max_depth), max_depth);

//     unsigned int max_num_threads = std::thread::hardware_concurrency();

//     uint8_t num_threads = 36;

//     std::thread *threads = new std::thread[num_threads];
//     std::vector<std::vector <int32_t>> *data_vector = get_data_vector();

//     for (uint8_t i = 0; i < num_threads; i++){
//         threads[i] = std::thread(test_osm_insert, mdtrie, data_vector, num_threads, i);
//     }

//     for (uint8_t i = 0; i < num_threads; i++){
//         threads[i].join();
//     }

//     int32_t data_vector_size = data_vector->size();

//     for (int i = 0; i < data_vector_size; i++){
//         std::vector <int32_t> point = (* data_vector)[i];
//         auto *leaf_point = new data_point();

//         for (uint8_t j = 0; j < DIMENSION; j++)
//             leaf_point->set_coordinate(j, point[j]);

//         if (!mdtrie->check(leaf_point)){
//             raise(SIGINT);
//             mdtrie->check(leaf_point);
//             std::cout << "index: " << i << " points not found!" << std::endl;
//         }        
//     }
// }

// TEST_CASE("Test random insert and reads", "[trie]") {

//     /**
//         Test concurrent reads and writes (5 reads and 5 writes)
//         See if there is error or deadlocks
//     */
//     create_level_to_num_children(std::vector<level_t>(DIMENSION, max_depth), max_depth);

//     auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);

//     unsigned int max_num_threads = std::thread::hardware_concurrency();

//     uint8_t num_threads = 10;

//     std::thread *threads = new std::thread[num_threads];

//     for (uint8_t i = 0; i < num_threads; i++){

//         if (i % 2 == 0)
//             threads[i] = std::thread(test_random_insert, mdtrie);
//         else
//             threads[i] = std::thread(test_random_read, mdtrie);
//     }

//     for (uint8_t i = 0; i < num_threads; i++){
//         threads[i].join();
//     }
// }

