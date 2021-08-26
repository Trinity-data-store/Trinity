#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <tqdm.h>
#include <vector>
#include <math.h> 
#include <iostream>
#include "elias_gamma_encoder.h"
#include "delta_encoded_array.h"

typedef unsigned long long int TimeStamp;

TimeStamp realloc_time = 0;

static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

void gamma_delta_push_from_scratch(uint64_t kArraySize){

  TimeStamp start = GetTimestamp();
  std::vector<uint64_t> array = {0};
  bitmap::EliasGammaDeltaEncodedArray<uint64_t> enc_array(array, 1);

  for (uint64_t i = 1; i < kArraySize; i++){
    // raise(SIGINT);
    enc_array.Push(i);
  }
  
  std::cout << "Elias Gamma Delta Encoded Array Insert Latency: " << (float) (GetTimestamp() - start) / kArraySize << "us per point" << std::endl;
  // std::cout << "Elias Gamma Delta Encoded Array Insert Realloc Time: " << (float)realloc_time / kArraySize << "us per point" << std::endl;
  // realloc_time = 0;

  // start = GetTimestamp();
  for (uint64_t i = 0; i < kArraySize; i++){
    if (enc_array[i] != i){
        std::cerr << "wrong!" << std::endl;
    }
  }  
  std::cout << "Elias Gamma Delta Encoded Array Read Latency: " << (float) (GetTimestamp() - start) / kArraySize << "us per point" << std::endl;
  // std::cout << "Elias Gamma Delta Encoded Array Read Realloc Time" << (float)realloc_time / kArraySize << "us per point" << std::endl;
}

void insert_vector_from_scratch(uint64_t kArraySize){

  TimeStamp start = GetTimestamp();
  std::vector<uint64_t> array;

  for (uint64_t i = 0; i < kArraySize; i++){
    array.push_back(i);
  }

  std::cout << "Insert Vector Insert Latency: " << (float) (GetTimestamp() - start) / kArraySize << "us per point" << std::endl;

  start = GetTimestamp();
  for (uint64_t i = 0; i < kArraySize; i++){
    if (array[i] != i){
        std::cerr << "wrong!" << std::endl;
    }
  }  
  std::cout << "Insert Vector Read Latency: " << (float) (GetTimestamp() - start) / kArraySize << "us per point" << std::endl;
}

bool binary_if_present(std::vector<uint64_t> *vect, uint64_t primary_key){
    uint64_t low = 0;
    uint64_t high = vect->size() - 1;

    while (low + 1 < high){
        uint64_t mid = (low + high) / 2;
        if ((*vect)[mid] < primary_key){
            low = mid;
        }
        else {
            high = mid;
        }
    }
    if ((*vect)[low] == primary_key || (*vect)[high] == primary_key){
        return true;
    }
    return false;
}

void gamma_delta_binary_search(uint64_t kArraySize){

  TimeStamp start = GetTimestamp();
  std::vector<uint64_t> array = {0};

  bitmap::EliasGammaDeltaEncodedArray<uint64_t> enc_array(array, array.size());

  for (uint64_t i = 10; i < kArraySize; i+= 10){
    // enc_array.Push(i);
    enc_array.Push(i);
    // array.push_back(i);
  }

  std::cout << "Elias Gamma Delta Encoded Array Insert Latency: " << (float) (GetTimestamp() - start) / (kArraySize / 10) << "us per point" << std::endl;

  start = GetTimestamp();
  for (uint64_t i = 10; i < kArraySize; i+= 10){
    // enc_array.Push(i);
    // enc_array.Push(i);
    array.push_back(i);
  }

  std::cout << "Vector Insert Latency: " << (float) (GetTimestamp() - start) / (kArraySize / 10) << "us per point" << std::endl;


  for (uint64_t i = 0; i < kArraySize; i+= 10){
    if (enc_array[i / 10] != i){
        std::cerr << "Not found first" << std::endl;
        raise(SIGINT);
        break;
    }
  } 

  start = GetTimestamp();
  for (uint64_t i = 0; i < kArraySize; i+= 10){

    if (!enc_array.Find(i)){
      std::cout << "Not found second" << std::endl;
      raise(SIGINT);
      break;
    }
  }

  std::cout << "Elias Gamma Delta Encoded Array Binary Search Latency: " << (float) (GetTimestamp() - start) / (kArraySize / 10) << "us per point" << std::endl;

  start = GetTimestamp();
  for (uint64_t i = 0; i < kArraySize; i+= 10){
    if (!enc_array.BinarySearch(i)){
      std::cout << "Not found third" << std::endl;
      raise(SIGINT);
    }
  }

  std::cout << "Elias Gamma Delta Encoded Array Binary Search Latency (Old): " << (float) (GetTimestamp() - start) / (kArraySize / 10) << "us per point" << std::endl;

  start = GetTimestamp();
  for (uint64_t i = 0; i < kArraySize; i+= 10){
    if (!binary_if_present(&array, i)){
      std::cout << "Not found fourth" << std::endl;
      raise(SIGINT);
    }
  }

  std::cout << "Vector Binary Search Latency: " << (float) (GetTimestamp() - start) / (kArraySize / 10) << "us per point" << std::endl;
  

  std::cout << "Size of delta encoded array: " << enc_array.size_overhead() << " bytes" << std::endl;
  std::cout << "Size of vector: " << sizeof(array) + sizeof(uint64_t) * array.size() << " bytes" << std::endl;

}

void test_vector_insertion(){

  std::vector<bitmap::EliasGammaDeltaEncodedArray<uint64_t>> primary_key_list;

  std::vector<uint64_t> array = {5};
  auto enc_array = bitmap::EliasGammaDeltaEncodedArray<uint64_t>(array, array.size());
  primary_key_list.emplace(primary_key_list.begin(), array, array.size());


  std::vector<uint64_t> array1 = {12};
  auto enc_array1 = bitmap::EliasGammaDeltaEncodedArray<uint64_t>(array1, array1.size());
  primary_key_list.emplace(primary_key_list.begin(), array1, array1.size());
}

int main() {

    // test_vector_insertion();
    // gamma_delta_push_from_scratch(10);
    // insert_vector_from_scratch(1000000);
    gamma_delta_binary_search(100);
}
