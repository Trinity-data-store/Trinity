#include "elias_gamma_encoder.h"
#include "delta_encoded_array.h"
#include <signal.h>
#include "catch.hpp"
#include "compact_ptr.h"

bool gamma_encoder_test() {

  std::vector<uint64_t> input;
  uint64_t kArraySize = 10000;
  for (uint64_t i = 0; i < kArraySize; i++) {
    input.push_back(i + 1);
  }

  auto encoded = bitmap::EliasGammaEncoder<uint64_t>::EncodeArray(input);
  auto decoded = bitmap::EliasGammaEncoder<uint64_t>::DecodeArray(encoded);

  for (uint64_t i = 0; i < kArraySize; i++) {
    if (decoded[i] != i + 1){
        return false;
    }
  }
  return true;
}

bool gamma_delta_encoder_test(){

  uint64_t kArraySize = 10000;
  std::vector<uint64_t> array;
  // uint64_t *array = new uint64_t[kArraySize];
  for (uint64_t i = 0; i < kArraySize; i++) {
    // array[i] = i;
    array.push_back(i);
  }

  bitmap::EliasGammaDeltaEncodedArray<uint64_t> enc_array(array, kArraySize);

  for (uint64_t i = 0; i < kArraySize; i++) {
    if (enc_array[i] != i){
      return false;
    }
  }
  return true;
}

bool gamma_delta_push_test(){

  uint64_t kArraySize = 10000;
  std::vector<uint64_t> array;

  for (uint64_t i = 0; i < kArraySize / 2; i++) {
    array.push_back(i);
  }

  bitmap::EliasGammaDeltaEncodedArray<uint64_t> enc_array(array, kArraySize / 2);

  for (uint64_t i = kArraySize / 2; i < kArraySize; i++){
    // if (i == 5031){
    //   raise(SIGINT);
    // }
    

    enc_array.Push(i);
  }

  for (uint64_t i = 0; i < kArraySize; i++) {
    if (enc_array[i] != i){
      raise(SIGINT);
      std::cout << enc_array[i];
      return false;
    }
  }
  return true;
}

bool gamma_delta_push_from_scratch_test(){

  uint64_t kArraySize = 100000;
  std::vector<uint64_t> array = {0};

  bitmap::EliasGammaDeltaEncodedArray<uint64_t> enc_array(array, 1);

  for (uint64_t i = 1; i < kArraySize; i++){
    // raise(SIGINT);
    enc_array.Push(i);
  }

  for (uint64_t i = 0; i < kArraySize; i++) {
    if (enc_array[i] != i){
      return false;
    }
  }
  return true;
}

bool gamma_delta_push_from_scratch_increment_test(){

  uint64_t kArraySize = 100000;
  std::vector<uint64_t> array = {0};

  bitmap::EliasGammaDeltaEncodedArray<uint64_t> enc_array(array, 1);

  for (uint64_t i = 10; i < kArraySize; i+= 10){
    // raise(SIGINT);
    enc_array.Push(i);
  }

  for (uint64_t i = 0; i < kArraySize; i+= 10) {
    if (enc_array[i / 10] != i){
      raise(SIGINT);
      return false;
    }
  }
  return true;
}

bool gamma_delta_binary_search_test(){

  uint64_t kArraySize = 100000;
  std::vector<uint64_t> array = {0};

  bitmap::EliasGammaDeltaEncodedArray<uint64_t> enc_array(array, 1);

  for (uint64_t i = 10; i < kArraySize; i+= 10){
    // raise(SIGINT);
    enc_array.Push(i);
  }

  for (uint64_t i = 0; i < kArraySize; i ++) {
    if (i % 10 == 0 && !enc_array.Find(i)){
      raise(SIGINT);
      return false;
    }
    if (i % 10 != 0 && enc_array.Find(i)){
      raise(SIGINT);
      return false;
    }
  }
  return true;
}

// bool compact_pointer_test(){


//   std::vector<bits::compact_ptr> primary_key_list;
//   std::vector<uint64_t> test_vect = {0, 1, 2, 3};
//   bits::compact_ptr vect_compact_ptr(&test_vect, test_vect.size());
//   std::vector<uint64_t> *test_vect_ptr = vect_compact_ptr.get_vector_pointer();
//   test_vect_ptr->push_back(5);

//   if (!vect_compact_ptr.check_is_vector() || test_vect.size() != 5){
//     return false;
//   }

//   bitmap::EliasGammaDeltaEncodedArray<uint64_t> enc_array(test_vect, test_vect.size());
//   bits::compact_ptr array_compact_ptr(&enc_array, test_vect.size());

//   bitmap::EliasGammaDeltaEncodedArray<uint64_t> *enc_array_ptr = array_compact_ptr.get_delta_encoded_array_pointer();
//   enc_array_ptr->Push(5);
//   if (enc_array[4] != 5){
//     return false;
//   }

//   primary_key_list.push_back(vect_compact_ptr);
//   primary_key_list.push_back(array_compact_ptr);
//   return true;
// }


TEST_CASE("Check Gamma Encoding Correctness", "[gamma encoder]") {

    REQUIRE(gamma_encoder_test());
}

TEST_CASE("Check Delta Encoding Correctness", "[delta encoder]") {

    REQUIRE(gamma_delta_encoder_test());
}

TEST_CASE("Check Push Correctness", "[delta encoder]") {

    REQUIRE(gamma_delta_push_test());
}

TEST_CASE("Check Push from scratch Correctness", "[delta encoder]") {

    REQUIRE(gamma_delta_push_from_scratch_test());
    REQUIRE(gamma_delta_push_from_scratch_increment_test());
}

TEST_CASE("Check Binary Search", "[delta encoder]") {

    REQUIRE(gamma_delta_binary_search_test());
}

// TEST_CASE("Check Compact Pointer Test", "[Compact Pointer]") {

//     REQUIRE(compact_pointer_test());
// }