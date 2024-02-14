#ifndef BITMAP_BITMAP_H_
#define BITMAP_BITMAP_H_

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <vector>

namespace bitmap
{

#define GETBIT(n, i) ((n >> i) & 1UL)
#define SETBIT(n, i) n = (n | (1UL << i))
#define CLRBIT(n, i) n = (n & ~(1UL << i))

#define BITS2BLOCKS(bits) \
  (((bits) % 64 == 0) ? ((bits) / 64) : (((bits) / 64) + 1))

#define GETBITVAL(data, i) GETBIT((data)[(i) / 64], (i) % 64)
#define SETBITVAL(data, i) SETBIT((data)[(i) / 64], (i) % 64)
#define CLRBITVAL(data, i) CLRBIT((data)[(i) / 64], (i) % 64)

  const uint64_t all_set = -1ULL;

  static const uint64_t high_bits_set[65] = {
      0x0000000000000000ULL, 0x8000000000000000ULL, 0xC000000000000000ULL,
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

  static const uint64_t high_bits_unset[65] = {
      0xFFFFFFFFFFFFFFFFULL, 0x7FFFFFFFFFFFFFFFULL, 0x3FFFFFFFFFFFFFFFULL,
      0x1FFFFFFFFFFFFFFFULL, 0x0FFFFFFFFFFFFFFFULL, 0x07FFFFFFFFFFFFFFULL,
      0x03FFFFFFFFFFFFFFULL, 0x01FFFFFFFFFFFFFFULL, 0x00FFFFFFFFFFFFFFULL,
      0x007FFFFFFFFFFFFFULL, 0x003FFFFFFFFFFFFFULL, 0x001FFFFFFFFFFFFFULL,
      0x000FFFFFFFFFFFFFULL, 0x0007FFFFFFFFFFFFULL, 0x0003FFFFFFFFFFFFULL,
      0x0001FFFFFFFFFFFFULL, 0x0000FFFFFFFFFFFFULL, 0x00007FFFFFFFFFFFULL,
      0x00003FFFFFFFFFFFULL, 0x00001FFFFFFFFFFFULL, 0x00000FFFFFFFFFFFULL,
      0x000007FFFFFFFFFFULL, 0x000003FFFFFFFFFFULL, 0x000001FFFFFFFFFFULL,
      0x000000FFFFFFFFFFULL, 0x0000007FFFFFFFFFULL, 0x0000003FFFFFFFFFULL,
      0x0000001FFFFFFFFFULL, 0x0000000FFFFFFFFFULL, 0x00000007FFFFFFFFULL,
      0x00000003FFFFFFFFULL, 0x00000001FFFFFFFFULL, 0x00000000FFFFFFFFULL,
      0x000000007FFFFFFFULL, 0x000000003FFFFFFFULL, 0x000000001FFFFFFFULL,
      0x000000000FFFFFFFULL, 0x0000000007FFFFFFULL, 0x0000000003FFFFFFULL,
      0x0000000001FFFFFFULL, 0x0000000000FFFFFFULL, 0x00000000007FFFFFULL,
      0x00000000003FFFFFULL, 0x00000000001FFFFFULL, 0x00000000000FFFFFULL,
      0x000000000007FFFFULL, 0x000000000003FFFFULL, 0x000000000001FFFFULL,
      0x000000000000FFFFULL, 0x0000000000007FFFULL, 0x0000000000003FFFULL,
      0x0000000000001FFFULL, 0x0000000000000FFFULL, 0x00000000000007FFULL,
      0x00000000000003FFULL, 0x00000000000001FFULL, 0x00000000000000FFULL,
      0x000000000000007FULL, 0x000000000000003FULL, 0x000000000000001FULL,
      0x000000000000000FULL, 0x0000000000000007ULL, 0x0000000000000003ULL,
      0x0000000000000001ULL, 0x0000000000000000ULL};

  static const uint64_t low_bits_set[65] = {
      0x0000000000000000ULL, 0x0000000000000001ULL, 0x0000000000000003ULL,
      0x0000000000000007ULL, 0x000000000000000FULL, 0x000000000000001FULL,
      0x000000000000003FULL, 0x000000000000007FULL, 0x00000000000000FFULL,
      0x00000000000001FFULL, 0x00000000000003FFULL, 0x00000000000007FFULL,
      0x0000000000000FFFULL, 0x0000000000001FFFULL, 0x0000000000003FFFULL,
      0x0000000000007FFFULL, 0x000000000000FFFFULL, 0x000000000001FFFFULL,
      0x000000000003FFFFULL, 0x000000000007FFFFULL, 0x00000000000FFFFFULL,
      0x00000000001FFFFFULL, 0x00000000003FFFFFULL, 0x00000000007FFFFFULL,
      0x0000000000FFFFFFULL, 0x0000000001FFFFFFULL, 0x0000000003FFFFFFULL,
      0x0000000007FFFFFFULL, 0x000000000FFFFFFFULL, 0x000000001FFFFFFFULL,
      0x000000003FFFFFFFULL, 0x000000007FFFFFFFULL, 0x00000000FFFFFFFFULL,
      0x00000001FFFFFFFFULL, 0x00000003FFFFFFFFULL, 0x00000007FFFFFFFFULL,
      0x0000000FFFFFFFFFULL, 0x0000001FFFFFFFFFULL, 0x0000003FFFFFFFFFULL,
      0x0000007FFFFFFFFFULL, 0x000000FFFFFFFFFFULL, 0x000001FFFFFFFFFFULL,
      0x000003FFFFFFFFFFULL, 0x000007FFFFFFFFFFULL, 0x00000FFFFFFFFFFFULL,
      0x00001FFFFFFFFFFFULL, 0x00003FFFFFFFFFFFULL, 0x00007FFFFFFFFFFFULL,
      0x0000FFFFFFFFFFFFULL, 0x0001FFFFFFFFFFFFULL, 0x0003FFFFFFFFFFFFULL,
      0x0007FFFFFFFFFFFFULL, 0x000FFFFFFFFFFFFFULL, 0x001FFFFFFFFFFFFFULL,
      0x003FFFFFFFFFFFFFULL, 0x007FFFFFFFFFFFFFULL, 0x00FFFFFFFFFFFFFFULL,
      0x01FFFFFFFFFFFFFFULL, 0x03FFFFFFFFFFFFFFULL, 0x07FFFFFFFFFFFFFFULL,
      0x0FFFFFFFFFFFFFFFULL, 0x1FFFFFFFFFFFFFFFULL, 0x3FFFFFFFFFFFFFFFULL,
      0x7FFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL};

  static const uint64_t low_bits_unset[65] = {
      0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFEULL, 0xFFFFFFFFFFFFFFFCULL,
      0xFFFFFFFFFFFFFFF8ULL, 0xFFFFFFFFFFFFFFF0ULL, 0xFFFFFFFFFFFFFFE0ULL,
      0xFFFFFFFFFFFFFFC0ULL, 0xFFFFFFFFFFFFFF80ULL, 0xFFFFFFFFFFFFFF00ULL,
      0xFFFFFFFFFFFFFE00ULL, 0xFFFFFFFFFFFFFC00ULL, 0xFFFFFFFFFFFFF800ULL,
      0xFFFFFFFFFFFFF000ULL, 0xFFFFFFFFFFFFE000ULL, 0xFFFFFFFFFFFFC000ULL,
      0xFFFFFFFFFFFF8000ULL, 0xFFFFFFFFFFFF0000ULL, 0xFFFFFFFFFFFE0000ULL,
      0xFFFFFFFFFFFC0000ULL, 0xFFFFFFFFFFF80000ULL, 0xFFFFFFFFFFF00000ULL,
      0xFFFFFFFFFFE00000ULL, 0xFFFFFFFFFFC00000ULL, 0xFFFFFFFFFF800000ULL,
      0xFFFFFFFFFF000000ULL, 0xFFFFFFFFFE000000ULL, 0xFFFFFFFFFC000000ULL,
      0xFFFFFFFFF8000000ULL, 0xFFFFFFFFF0000000ULL, 0xFFFFFFFFE0000000ULL,
      0xFFFFFFFFC0000000ULL, 0xFFFFFFFF80000000ULL, 0xFFFFFFFF00000000ULL,
      0xFFFFFFFE00000000ULL, 0xFFFFFFFC00000000ULL, 0xFFFFFFF800000000ULL,
      0xFFFFFFF000000000ULL, 0xFFFFFFE000000000ULL, 0xFFFFFFC000000000ULL,
      0xFFFFFF8000000000ULL, 0xFFFFFF0000000000ULL, 0xFFFFFE0000000000ULL,
      0xFFFFFC0000000000ULL, 0xFFFFF80000000000ULL, 0xFFFFF00000000000ULL,
      0xFFFFE00000000000ULL, 0xFFFFC00000000000ULL, 0xFFFF800000000000ULL,
      0xFFFF000000000000ULL, 0xFFFE000000000000ULL, 0xFFFC000000000000ULL,
      0xFFF8000000000000ULL, 0xFFF0000000000000ULL, 0xFFE0000000000000ULL,
      0xFFC0000000000000ULL, 0xFF80000000000000ULL, 0xFF00000000000000ULL,
      0xFE00000000000000ULL, 0xFC00000000000000ULL, 0xF800000000000000ULL,
      0xF000000000000000ULL, 0xE000000000000000ULL, 0xC000000000000000ULL,
      0x8000000000000000ULL, 0x0000000000000000ULL};

  class Bitmap
  {
  public:
    // Type definitions
    typedef size_t pos_type;
    typedef size_t size_type;
    typedef uint64_t data_type;
    typedef uint8_t width_type;

    // Constructors and Destructors
    Bitmap() { size_ = 0; }

    Bitmap(size_type num_bits) { Bitmap_Init(num_bits); }

    void Bitmap_Init(size_type num_bits)
    {

      data_ = static_cast<data_type *>(
          malloc(BITS2BLOCKS(num_bits) * sizeof(data_type)));
      size_ = num_bits;
    }

    void Resize(size_type num_bits)
    {
      size_type target = BITS2BLOCKS(num_bits);
      if (target > BITS2BLOCKS(size_))
        data_ =
            static_cast<data_type *>(realloc(data_, target * sizeof(data_type)));
      size_ = num_bits;
    }

    void Realloc_increase(size_type num_bits) { Resize(size_ + num_bits); }

    virtual ~Bitmap() = default;

    size_type GetSizeInBits() { return size_; }

    size_type GetSize()
    {
      return sizeof(size_t) + sizeof(data_type *) +
             sizeof(data_type) * BITS2BLOCKS(size_);
    }

    void Clear()
    {
      memset((void *)data_, 0, BITS2BLOCKS(size_) * sizeof(uint64_t));
    }

    void SetBit(pos_type i) { SETBITVAL(data_, i); }

    void UnsetBit(pos_type i) { CLRBITVAL(data_, i); }

    bool GetBit(pos_type i) const { return GETBITVAL(data_, i); }

    // Integer operations
    void SetValPos(pos_type pos, data_type val, width_type bits)
    {
      pos_type s_off = pos % 64;
      pos_type s_idx = pos / 64;

      if (s_off + bits <= 64)
      {
        // Can be accommodated in 1 bitmap block
        data_[s_idx] =
            (data_[s_idx] & (low_bits_set[s_off] | low_bits_unset[s_off + bits])) |
            val << s_off;
      }
      else
      {
        // Must use 2 bitmap blocks
        data_[s_idx] = (data_[s_idx] & low_bits_set[s_off]) | val << s_off;
        data_[s_idx + 1] =
            (data_[s_idx + 1] & low_bits_unset[(s_off + bits) % 64]) |
            (val >> (64 - s_off));
      }
    }

    data_type GetValPos(pos_type pos, width_type bits) const
    {
      pos_type s_off = pos % 64;
      pos_type s_idx = pos / 64;

      if (s_off + bits <= 64)
      {
        // Can be read from a single block
        return (data_[s_idx] >> s_off) & low_bits_set[bits];
      }
      else
      {
        // Must be read from two blocks
        return ((data_[s_idx] >> s_off) | (data_[s_idx + 1] << (64 - s_off))) &
               low_bits_set[bits];
      }
    }

    // Serialization/De-serialization
    virtual size_type Serialize(std::ostream &out)
    {
      size_t out_size = 0;

      out.write(reinterpret_cast<const char *>(&size_), sizeof(size_type));
      out_size += sizeof(size_type);

      for (uint64_t i = 0; i < BITS2BLOCKS(size_); i++)
      {
        out.write(reinterpret_cast<const char *>(&data_[i]), sizeof(data_type));
        out_size += sizeof(data_type);
      }

      // out.write(reinterpret_cast<const char *>(data_),
      //           sizeof(data_type) * BITS2BLOCKS(size_));
      // out_size += (BITS2BLOCKS(size_) * sizeof(uint64_t));

      return out_size;
    }

    virtual size_type Deserialize(std::istream &in)
    {
      size_t in_size = 0;

      in.read(reinterpret_cast<char *>(&size_), sizeof(size_type));
      in_size += sizeof(size_type);

      data_ = new data_type[BITS2BLOCKS(size_)];

      for (uint64_t i = 0; i < BITS2BLOCKS(size_); i++)
      {
        in.read(reinterpret_cast<char *>(&data_[i]), sizeof(data_type));
        in_size += sizeof(data_type);
      }
      // in.read(reinterpret_cast<char *>(data_),
      // BITS2BLOCKS(size_) * sizeof(data_type));
      // in_size += (BITS2BLOCKS(size_) * sizeof(data_type));

      return in_size;
    }

  protected:
    // Data members
    data_type *data_{};
    size_type size_{};
  };

} // namespace bitmap

#endif