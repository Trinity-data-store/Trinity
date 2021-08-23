#include "elias_gamma_encoder.h"
#include "delta_encoded_array.h"
#include <signal.h>
#include "catch.hpp"

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

  bitmap::EliasGammaDeltaEncodedArray<uint64_t> enc_array(array, kArraySize);

  for (uint64_t i = kArraySize / 2; i < kArraySize; i++){
    raise(SIGINT);
    enc_array.Push(i);
  }

  for (uint64_t i = 0; i < kArraySize; i++) {
    if (enc_array[i] != i){
      return false;
    }
  }
  return true;
}

TEST_CASE("Check Gamma Encoding Correctness", "[gamma encoder]") {

    REQUIRE(gamma_encoder_test());
}

TEST_CASE("Check Delta Encoding Correctness", "[delta encoder]") {

    REQUIRE(gamma_delta_encoder_test());
}

TEST_CASE("Check Push Correctness", "[delta encoder]") {

    REQUIRE(gamma_delta_push_test());
}
