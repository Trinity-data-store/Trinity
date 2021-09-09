#ifndef ELIAS_GAMMA_ENCODED_ARRAY_H_
#define ELIAS_GAMMA_ENCODED_ARRAY_H_

#include <vector>
#include <iostream>

#include "bitmap.h"
#include "bitmap_array.h"
#include "utils.h"

namespace bitmap {

template<typename T>
class EliasGammaEncoder {
 public:
  typedef size_t size_type;
  typedef size_t pos_type;
  typedef uint8_t width_type;

  static width_type EncodingSize(T val) {
    return 2 * (Utils::BitWidth(val) - 1) + 1;
  }

  static Bitmap* EncodeArray(std::vector<T> &vals) {
    size_type out_size = 0;
    for (size_t i = 0; i < vals.size(); i++) {
      assert(vals[i] > 0);
      out_size += EncodingSize(vals[i]);
    }
    auto* out = new Bitmap(out_size);
    uint64_t pos = 0;
    for (size_t i = 0; i < vals.size(); i++) {
      uint64_t nbits = Utils::BitWidth(vals[i]) - 1;
      pos += nbits;
      assert(vals[i] == 0 || (1ULL << nbits) <= vals[i]);
      out->SetBit(pos++);
      out->SetValPos(pos, vals[i] - (1ULL << nbits), nbits);
      pos += nbits;
    }
    return out;
  }

  static std::vector<T> DecodeArray(Bitmap* encoded) {
    std::vector<T> out;
    size_type off = 0;
    auto max_off = encoded->GetSizeInBits();
    while (off != max_off) {
      width_type val_width = 0;
      while (!encoded->GetBit(off)) {
        val_width++;
        off++;
      }
      off++;
      T decoded = encoded->GetValPos(off, val_width) + (1ULL << val_width);
      out.push_back(decoded);
      off += val_width;
    }
    return out;
  }
};

}

#endif // ELIAS_GAMMA_ENCODED_ARRAY_H_