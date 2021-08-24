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

void gamma_delta_binary_search(uint64_t kArraySize){

  TimeStamp start = GetTimestamp();
  std::vector<uint64_t> array = {0};

  bitmap::EliasGammaDeltaEncodedArray<uint64_t> enc_array(array, array.size());

  for (uint64_t i = 10; i < kArraySize; i+= 10){
    // enc_array.Push(i);
    enc_array.Push(i);
  }

  // bitmap::EliasGammaDeltaEncodedArray<uint64_t> enc_array(array, array.size());

  std::cout << "Elias Gamma Delta Encoded Array Insert Latency: " << (float) (GetTimestamp() - start) / (kArraySize / 10) << "us per point" << std::endl;

  // for (uint64_t i = 0; i < 200; i++){
  //   if (enc_array[i] == 242){
  //       std::cerr << "found 242" << std::endl;
  //       break;
  //   }
  // } 
  start = GetTimestamp();
  for (uint64_t i = 0; i < kArraySize; i+= 10){
    if (!enc_array.BinarySearch(i)){
      std::cout << "Not found" << std::endl;
      raise(SIGINT);
    }
  }

  std::cout << "Elias Gamma Delta Encoded Array Binary Search Latency: " << (float) (GetTimestamp() - start) / (kArraySize / 10) << "us per point" << std::endl;

  
}

int main() {
    gamma_delta_push_from_scratch(1000000);
    // insert_vector_from_scratch(1000000);
    gamma_delta_binary_search(1000000);
}
