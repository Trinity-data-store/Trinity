#ifndef BITMAP_UTILS_H_
#define BITMAP_UTILS_H_

namespace bitmap {

class Utils {
 public:
  static uint8_t BitWidth(uint64_t n) {
    uint8_t l = 1;
    while (n >>= 1)
      ++l;
    return l;
  }

  static uint8_t Popcount64bit(uint64_t n) {
    return __builtin_popcountll(n);
  }

  static uint16_t Popcount512bit(uint64_t *data) {
    return __builtin_popcountll(*data) + __builtin_popcountll(*(data + 1))
        + __builtin_popcountll(*(data + 2)) + __builtin_popcountll(*(data + 3))
        + __builtin_popcountll(*(data + 4)) + __builtin_popcountll(*(data + 5))
        + __builtin_popcountll(*(data + 6)) + __builtin_popcountll(*(data + 7));
  }
};

}

#endif // BITMAP_UTILS_H_