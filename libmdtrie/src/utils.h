#ifndef BITMAP_UTILS_H_
#define BITMAP_UTILS_H_


#define GETBIT(n, i)    ((n >> i) & 1UL)
#define SETBIT(n, i)    n = (n | (1UL << i))
#define CLRBIT(n, i)  n = (n & ~(1UL << i))

#define BITS2BLOCKS(bits) \
    (((bits) % 64 == 0) ? ((bits) / 64) : (((bits) / 64) + 1))

#define GETBITVAL(data, i) GETBIT((data)[(i) / 64], (i) % 64)
#define SETBITVAL(data, i) SETBIT((data)[(i) / 64], (i) % 64)
#define CLRBITVAL(data, i) CLRBIT((data)[(i) / 64], (i) % 64)

const uint64_t all_set = -1ULL;

static const uint64_t high_bits_set[65] = {0x0000000000000000ULL, 0x8000000000000000ULL, 0xC000000000000000ULL,
                                           0xE000000000000000ULL, 0xF000000000000000ULL, 0xF800000000000000ULL,
                                           0xFC00000000000000ULL, 0xFE00000000000000ULL, 0xFF00000000000000ULL,
                                           0xFF80000000000000ULL, 0xFFC0000000000000ULL, 0xFFE0000000000000ULL,
                                           0xFFF0000000000000ULL, 0xFFF8000000000000ULL, 0xFFFC000000000000ULL,
                                           0xFFFE000000000000ULL, 0xFFFF000000000000ULL, 0xFFFF800000000000ULL,
                                           0xFFFFC00000000000ULL, 0xFFFFE00000000000ULL, 0xFFFFF00000000000ULL,
                                           0xFFFFF80000000000ULL, 0xFFFFFC0000000000ULL, 0xFFFFFE0000000000ULL,
                                           0xFFFFFF0000000000ULL, 0xFFFFFF8000000000ULL, 0xFFFFFFC000000000ULL,
                                           0xFFFFFFE000000000ULL, 0xFFFFFFF000000000ULL, 0xFFFFFFF800000000ULL,
                                           0xFFFFFFFC00000000ULL, 0xFFFFFFFE00000000ULL, 0xFFFFFFFF00000000ULL,
                                           0xFFFFFFFF80000000ULL, 0xFFFFFFFFC0000000ULL, 0xFFFFFFFFE0000000ULL,
                                           0xFFFFFFFFF0000000ULL, 0xFFFFFFFFF8000000ULL, 0xFFFFFFFFFC000000ULL,
                                           0xFFFFFFFFFE000000ULL, 0xFFFFFFFFFF000000ULL, 0xFFFFFFFFFF800000ULL,
                                           0xFFFFFFFFFFC00000ULL, 0xFFFFFFFFFFE00000ULL, 0xFFFFFFFFFFF00000ULL,
                                           0xFFFFFFFFFFF80000ULL, 0xFFFFFFFFFFFC0000ULL, 0xFFFFFFFFFFFE0000ULL,
                                           0xFFFFFFFFFFFF0000ULL, 0xFFFFFFFFFFFF8000ULL, 0xFFFFFFFFFFFFC000ULL,
                                           0xFFFFFFFFFFFFE000ULL, 0xFFFFFFFFFFFFF000ULL, 0xFFFFFFFFFFFFF800ULL,
                                           0xFFFFFFFFFFFFFC00ULL, 0xFFFFFFFFFFFFFE00ULL, 0xFFFFFFFFFFFFFF00ULL,
                                           0xFFFFFFFFFFFFFF80ULL, 0xFFFFFFFFFFFFFFC0ULL, 0xFFFFFFFFFFFFFFE0ULL,
                                           0xFFFFFFFFFFFFFFF0ULL, 0xFFFFFFFFFFFFFFF8ULL, 0xFFFFFFFFFFFFFFFCULL,
                                           0xFFFFFFFFFFFFFFFEULL, 0xFFFFFFFFFFFFFFFFULL};

static const uint64_t high_bits_unset[65] = {0xFFFFFFFFFFFFFFFFULL,
                                             0x7FFFFFFFFFFFFFFFULL, 0x3FFFFFFFFFFFFFFFULL, 0x1FFFFFFFFFFFFFFFULL,
                                             0x0FFFFFFFFFFFFFFFULL, 0x07FFFFFFFFFFFFFFULL, 0x03FFFFFFFFFFFFFFULL,
                                             0x01FFFFFFFFFFFFFFULL, 0x00FFFFFFFFFFFFFFULL, 0x007FFFFFFFFFFFFFULL,
                                             0x003FFFFFFFFFFFFFULL, 0x001FFFFFFFFFFFFFULL, 0x000FFFFFFFFFFFFFULL,
                                             0x0007FFFFFFFFFFFFULL, 0x0003FFFFFFFFFFFFULL, 0x0001FFFFFFFFFFFFULL,
                                             0x0000FFFFFFFFFFFFULL, 0x00007FFFFFFFFFFFULL, 0x00003FFFFFFFFFFFULL,
                                             0x00001FFFFFFFFFFFULL, 0x00000FFFFFFFFFFFULL, 0x000007FFFFFFFFFFULL,
                                             0x000003FFFFFFFFFFULL, 0x000001FFFFFFFFFFULL, 0x000000FFFFFFFFFFULL,
                                             0x0000007FFFFFFFFFULL, 0x0000003FFFFFFFFFULL, 0x0000001FFFFFFFFFULL,
                                             0x0000000FFFFFFFFFULL, 0x00000007FFFFFFFFULL, 0x00000003FFFFFFFFULL,
                                             0x00000001FFFFFFFFULL, 0x00000000FFFFFFFFULL, 0x000000007FFFFFFFULL,
                                             0x000000003FFFFFFFULL, 0x000000001FFFFFFFULL, 0x000000000FFFFFFFULL,
                                             0x0000000007FFFFFFULL, 0x0000000003FFFFFFULL, 0x0000000001FFFFFFULL,
                                             0x0000000000FFFFFFULL, 0x00000000007FFFFFULL, 0x00000000003FFFFFULL,
                                             0x00000000001FFFFFULL, 0x00000000000FFFFFULL, 0x000000000007FFFFULL,
                                             0x000000000003FFFFULL, 0x000000000001FFFFULL, 0x000000000000FFFFULL,
                                             0x0000000000007FFFULL, 0x0000000000003FFFULL, 0x0000000000001FFFULL,
                                             0x0000000000000FFFULL, 0x00000000000007FFULL, 0x00000000000003FFULL,
                                             0x00000000000001FFULL, 0x00000000000000FFULL, 0x000000000000007FULL,
                                             0x000000000000003FULL, 0x000000000000001FULL, 0x000000000000000FULL,
                                             0x0000000000000007ULL, 0x0000000000000003ULL, 0x0000000000000001ULL,
                                             0x0000000000000000ULL};

static const uint64_t low_bits_set[65] = {0x0000000000000000ULL,
                                          0x0000000000000001ULL, 0x0000000000000003ULL, 0x0000000000000007ULL,
                                          0x000000000000000FULL, 0x000000000000001FULL, 0x000000000000003FULL,
                                          0x000000000000007FULL, 0x00000000000000FFULL, 0x00000000000001FFULL,
                                          0x00000000000003FFULL, 0x00000000000007FFULL, 0x0000000000000FFFULL,
                                          0x0000000000001FFFULL, 0x0000000000003FFFULL, 0x0000000000007FFFULL,
                                          0x000000000000FFFFULL, 0x000000000001FFFFULL, 0x000000000003FFFFULL,
                                          0x000000000007FFFFULL, 0x00000000000FFFFFULL, 0x00000000001FFFFFULL,
                                          0x00000000003FFFFFULL, 0x00000000007FFFFFULL, 0x0000000000FFFFFFULL,
                                          0x0000000001FFFFFFULL, 0x0000000003FFFFFFULL, 0x0000000007FFFFFFULL,
                                          0x000000000FFFFFFFULL, 0x000000001FFFFFFFULL, 0x000000003FFFFFFFULL,
                                          0x000000007FFFFFFFULL, 0x00000000FFFFFFFFULL, 0x00000001FFFFFFFFULL,
                                          0x00000003FFFFFFFFULL, 0x00000007FFFFFFFFULL, 0x0000000FFFFFFFFFULL,
                                          0x0000001FFFFFFFFFULL, 0x0000003FFFFFFFFFULL, 0x0000007FFFFFFFFFULL,
                                          0x000000FFFFFFFFFFULL, 0x000001FFFFFFFFFFULL, 0x000003FFFFFFFFFFULL,
                                          0x000007FFFFFFFFFFULL, 0x00000FFFFFFFFFFFULL, 0x00001FFFFFFFFFFFULL,
                                          0x00003FFFFFFFFFFFULL, 0x00007FFFFFFFFFFFULL, 0x0000FFFFFFFFFFFFULL,
                                          0x0001FFFFFFFFFFFFULL, 0x0003FFFFFFFFFFFFULL, 0x0007FFFFFFFFFFFFULL,
                                          0x000FFFFFFFFFFFFFULL, 0x001FFFFFFFFFFFFFULL, 0x003FFFFFFFFFFFFFULL,
                                          0x007FFFFFFFFFFFFFULL, 0x00FFFFFFFFFFFFFFULL, 0x01FFFFFFFFFFFFFFULL,
                                          0x03FFFFFFFFFFFFFFULL, 0x07FFFFFFFFFFFFFFULL, 0x0FFFFFFFFFFFFFFFULL,
                                          0x1FFFFFFFFFFFFFFFULL, 0x3FFFFFFFFFFFFFFFULL, 0x7FFFFFFFFFFFFFFFULL,
                                          0xFFFFFFFFFFFFFFFFULL};

static const uint64_t low_bits_unset[65] = {0xFFFFFFFFFFFFFFFFULL,
                                            0xFFFFFFFFFFFFFFFEULL, 0xFFFFFFFFFFFFFFFCULL, 0xFFFFFFFFFFFFFFF8ULL,
                                            0xFFFFFFFFFFFFFFF0ULL, 0xFFFFFFFFFFFFFFE0ULL, 0xFFFFFFFFFFFFFFC0ULL,
                                            0xFFFFFFFFFFFFFF80ULL, 0xFFFFFFFFFFFFFF00ULL, 0xFFFFFFFFFFFFFE00ULL,
                                            0xFFFFFFFFFFFFFC00ULL, 0xFFFFFFFFFFFFF800ULL, 0xFFFFFFFFFFFFF000ULL,
                                            0xFFFFFFFFFFFFE000ULL, 0xFFFFFFFFFFFFC000ULL, 0xFFFFFFFFFFFF8000ULL,
                                            0xFFFFFFFFFFFF0000ULL, 0xFFFFFFFFFFFE0000ULL, 0xFFFFFFFFFFFC0000ULL,
                                            0xFFFFFFFFFFF80000ULL, 0xFFFFFFFFFFF00000ULL, 0xFFFFFFFFFFE00000ULL,
                                            0xFFFFFFFFFFC00000ULL, 0xFFFFFFFFFF800000ULL, 0xFFFFFFFFFF000000ULL,
                                            0xFFFFFFFFFE000000ULL, 0xFFFFFFFFFC000000ULL, 0xFFFFFFFFF8000000ULL,
                                            0xFFFFFFFFF0000000ULL, 0xFFFFFFFFE0000000ULL, 0xFFFFFFFFC0000000ULL,
                                            0xFFFFFFFF80000000ULL, 0xFFFFFFFF00000000ULL, 0xFFFFFFFE00000000ULL,
                                            0xFFFFFFFC00000000ULL, 0xFFFFFFF800000000ULL, 0xFFFFFFF000000000ULL,
                                            0xFFFFFFE000000000ULL, 0xFFFFFFC000000000ULL, 0xFFFFFF8000000000ULL,
                                            0xFFFFFF0000000000ULL, 0xFFFFFE0000000000ULL, 0xFFFFFC0000000000ULL,
                                            0xFFFFF80000000000ULL, 0xFFFFF00000000000ULL, 0xFFFFE00000000000ULL,
                                            0xFFFFC00000000000ULL, 0xFFFF800000000000ULL, 0xFFFF000000000000ULL,
                                            0xFFFE000000000000ULL, 0xFFFC000000000000ULL, 0xFFF8000000000000ULL,
                                            0xFFF0000000000000ULL, 0xFFE0000000000000ULL, 0xFFC0000000000000ULL,
                                            0xFF80000000000000ULL, 0xFF00000000000000ULL, 0xFE00000000000000ULL,
                                            0xFC00000000000000ULL, 0xF800000000000000ULL, 0xF000000000000000ULL,
                                            0xE000000000000000ULL, 0xC000000000000000ULL, 0x8000000000000000ULL,
                                            0x0000000000000000ULL};
                                            
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