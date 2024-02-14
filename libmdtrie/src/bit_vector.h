#ifndef BITMAP_BIT_VECTOR_H_
#define BITMAP_BIT_VECTOR_H_

#include "utils.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace bitmap
{

  class BitVector
  {
  public:
    typedef size_t pos_type;
    typedef size_t size_type;
    typedef uint64_t data_type;
    typedef uint8_t width_type;

    BitVector()
        : data_(nullptr), size_(0)
    {
    }

    explicit BitVector(size_type num_bits) { Init(num_bits); }

    BitVector(data_type *data, size_type num_bits)
    {
      data_ = data;
      size_ = num_bits;
    }

    virtual ~BitVector() { Destroy(); }

    void Init(size_type num_bits)
    {
      data_ = static_cast<data_type *>(
          malloc(BITS2BLOCKS(num_bits) * sizeof(data_type)));
      size_ = num_bits;
    }

    void Destroy()
    {
      if (data_ != nullptr)
      {
        free(data_);
        data_ = nullptr;
      }
      size_ = 0;
    }

    void Resize(size_type num_bits)
    {
      size_type target = BITS2BLOCKS(num_bits);
      if (target > BITS2BLOCKS(size_))
        data_ =
            static_cast<data_type *>(realloc(data_, target * sizeof(data_type)));
      size_ = num_bits;
    }

    void GrowBy(size_type num_bits) { Resize(size_ + num_bits); }

    // Getters
    data_type *GetData() { return data_; }

    size_type GetSizeInBits() const { return size_; }

    uint64_t size_overhead() const
    {

      return sizeof(size_) + sizeof(data_) +
             sizeof(data_type) * BITS2BLOCKS(size_);
    }

    // Bit operations
    void Clear()
    {
      memset((void *)data_, 0, BITS2BLOCKS(size_) * sizeof(uint64_t));
    }

    void SetBit(pos_type i) { SETBITVAL(data_, i); }

    void UnsetBit(pos_type i) { CLRBITVAL(data_, i); }

    bool GetBit(pos_type i) const { return GETBITVAL(data_, i); }

    // Integer operations
    void AppendVal(data_type val, width_type bits)
    {
      pos_type pos = size_;
      GrowBy(bits);
      SetValPos(pos, val, bits);
    }

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
      // raise(SIGINT);
      out.write(reinterpret_cast<const char *>(&size_), sizeof(size_type));
      out_size += sizeof(size_type);

      for (uint64_t i = 0; i < BITS2BLOCKS(size_); i++)
      {
        out.write(reinterpret_cast<const char *>(&data_[i]), sizeof(data_type));
        out_size += sizeof(data_type);
      }

      // out.write(reinterpret_cast<const char *>(data_), sizeof(data_type) *
      // BITS2BLOCKS(size_)); out_size += (BITS2BLOCKS(size_) * sizeof(uint64_t));

      return out_size;
    }

    virtual size_type Deserialize(std::istream &in)
    {
      size_t in_size = 0;
      // raise(SIGINT);
      in.read(reinterpret_cast<char *>(&size_), sizeof(size_type));
      in_size += sizeof(size_type);

      data_ =
          static_cast<data_type *>(malloc(BITS2BLOCKS(size_) * sizeof(data_type)));

      for (uint64_t i = 0; i < BITS2BLOCKS(size_); i++)
      {
        in.read(reinterpret_cast<char *>(&data_[i]), sizeof(data_type));
        in_size += sizeof(data_type);
      }
      // in.read(reinterpret_cast<char *>(data_), BITS2BLOCKS(size_) *
      // sizeof(data_type)); in_size += (BITS2BLOCKS(size_) * sizeof(data_type));

      return in_size;
    }

  protected:
    // Data members
    data_type *data_{};
    size_type size_{};
  };

} // namespace bitmap

#endif