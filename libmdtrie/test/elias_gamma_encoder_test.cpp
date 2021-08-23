#include "elias_gamma_encoder.h"
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

TEST_CASE("Check Correctness", "[gamma encoder]") {

    REQUIRE(gamma_encoder_test());
}
