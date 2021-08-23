#ifndef BITMAP_BITMAP_H_
#define BITMAP_BITMAP_H_

#include <cstdint>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace bitmap {

#define GETBIT(n, i)    ((n >> i) & 1UL)
#define SETBIT(n, i)    n = (n | (1UL << i))
#define CLRBIT(n, i)  n = (n & ~(1UL << i))

#define BITS2BLOCKS(bits) \
    (((bits) % 64 == 0) ? ((bits) / 64) : (((bits) / 64) + 1))

#define GETBITVAL(data, i) GETBIT((data)[(i) / 64], (i) % 64)
#define SETBITVAL(data, i) SETBIT((data)[(i) / 64], (i) % 64)
#define CLRBITVAL(data, i) CLRBIT((data)[(i) / 64], (i) % 64)

const uint64_t all_set = -1ULL;

static const uint64_t high_bits_set[65] = { 0x0000000000000000ULL,
    0x8000000000000000ULL, 0xC000000000000000ULL, 0xE000000000000000ULL,
    0xF000000000000000ULL, 0xF800000000000000ULL, 0xFC00000000000000ULL,
    0xFE00000000000000ULL, 0xFF00000000000000ULL, 0xFF80000000000000ULL,
    0xFFC0000000000000ULL, 0xFFE0000000000000ULL, 0xFFF0000000000000ULL,
    0xFFF8000000000000ULL, 0xFFFC000000000000ULL, 0xFFFE000000000000ULL,
    0xFFFF000000000000ULL, 0xFFFF800000000000ULL, 0xFFFFC00000000000ULL,
    0xFFFFE00000000000ULL, 0xFFFFF00000000000ULL, 0xFFFFF80000000000ULL,
    0xFFFFFC0000000000ULL, 0xFFFFFE0000000000ULL, 0xFFFFFF0000000000ULL,
    0xFFFFFF8000000000ULL, 0xFFFFFFC000000000ULL, 0xFFFFFFE000000000ULL,
    0xFFFFFFF000000000ULL, 0xFFFFFFF800000000ULL, 0xFFFFFFFC00000000ULL,
    0xFFFFFFFE00000000ULL, 0xFFFFFFFF00000000ULL, 0xFFFFFFFF80000000ULL,
    0xFFFFFFFFC0000000ULL, 0xFFFFFFFFE0000000ULL, 0xFFFFFFFFF0000000ULL,
    0xFFFFFFFFF8000000ULL, 0xFFFFFFFFFC000000ULL, 0xFFFFFFFFFE000000ULL,
    0xFFFFFFFFFF000000ULL, 0xFFFFFFFFFF800000ULL, 0xFFFFFFFFFFC00000ULL,
    0xFFFFFFFFFFE00000ULL, 0xFFFFFFFFFFF00000ULL, 0xFFFFFFFFFFF80000ULL,
    0xFFFFFFFFFFFC0000ULL, 0xFFFFFFFFFFFE0000ULL, 0xFFFFFFFFFFFF0000ULL,
    0xFFFFFFFFFFFF8000ULL, 0xFFFFFFFFFFFFC000ULL, 0xFFFFFFFFFFFFE000ULL,
    0xFFFFFFFFFFFFF000ULL, 0xFFFFFFFFFFFFF800ULL, 0xFFFFFFFFFFFFFC00ULL,
    0xFFFFFFFFFFFFFE00ULL, 0xFFFFFFFFFFFFFF00ULL, 0xFFFFFFFFFFFFFF80ULL,
    0xFFFFFFFFFFFFFFC0ULL, 0xFFFFFFFFFFFFFFE0ULL, 0xFFFFFFFFFFFFFFF0ULL,
    0xFFFFFFFFFFFFFFF8ULL, 0xFFFFFFFFFFFFFFFCULL, 0xFFFFFFFFFFFFFFFEULL,
    0xFFFFFFFFFFFFFFFFULL };

static const uint64_t high_bits_unset[65] = { 0xFFFFFFFFFFFFFFFFULL,
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
    0x0000000000000000ULL };

static const uint64_t low_bits_set[65] = { 0x0000000000000000ULL,
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
    0xFFFFFFFFFFFFFFFFULL };

static const uint64_t low_bits_unset[65] = { 0xFFFFFFFFFFFFFFFFULL,
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
    0x0000000000000000ULL };

class Bitmap {
 public:
  // Type definitions
  typedef size_t pos_type;
  typedef size_t size_type;
  typedef uint64_t data_type;
  typedef uint8_t width_type;

  // Constructors and Destructors
  Bitmap() {
    data_ = NULL;
    size_ = 0;
  }

  Bitmap(size_type num_bits) {
    data_ = new data_type[BITS2BLOCKS(num_bits)]();
    size_ = num_bits;
  }

  virtual ~Bitmap() {
    if (data_ != NULL) {
      delete[] data_;
      data_ = NULL;
    }
  }

  // Getters
  data_type* GetData() {
    return data_;
  }

  size_type GetSizeInBits() {
    return size_;
  }

  // Bit operations
  void Clear() {
    memset((void *) data_, 0, BITS2BLOCKS(size_) * sizeof(uint64_t));
  }

  void SetBit(pos_type i) {
    SETBITVAL(data_, i);
  }

  void UnsetBit(pos_type i) {
    CLRBITVAL(data_, i);
  }

  bool GetBit(pos_type i) const {
    return GETBITVAL(data_, i);
  }

  // Integer operations
  void SetValPos(pos_type pos, data_type val, width_type bits) {
    pos_type s_off = pos % 64;
    pos_type s_idx = pos / 64;

    if (s_off + bits <= 64) {
      // Can be accommodated in 1 bitmap block
      data_[s_idx] = (data_[s_idx]
          & (low_bits_set[s_off] | low_bits_unset[s_off + bits]))
          | val << s_off;
    } else {
      // Must use 2 bitmap blocks
      data_[s_idx] = (data_[s_idx] & low_bits_set[s_off]) | val << s_off;
      data_[s_idx + 1] =
          (data_[s_idx + 1] & low_bits_unset[(s_off + bits) % 64])
              | (val >> (64 - s_off));
    }
  }

  data_type GetValPos(pos_type pos, width_type bits) const {
    pos_type s_off = pos % 64;
    pos_type s_idx = pos / 64;

    if (s_off + bits <= 64) {
      // Can be read from a single block
      return (data_[s_idx] >> s_off) & low_bits_set[bits];
    } else {
      // Must be read from two blocks
<<<<<<< HEAD
      return ((data_[s_idx] >> s_off) | (data_[s_idx + 1] << (64 - s_off)))
          & low_bits_set[bits];
=======
      if (is_on_data){
        return ((data_[s_idx] >> s_off) | (data_[s_idx + 1] << (64 - s_off)))
            & low_bits_set[bits];
      }
      else {
        return ((flag_[s_idx] >> s_off) | (flag_[s_idx + 1] << (64 - s_off)))
            & low_bits_set[bits];        
      }
    }
  }

  inline void ClearWidth(pos_type pos, width_type width, bool is_on_data)
  {  
    if (width <= 64){
      SetValPos(pos, 0, width, is_on_data);
      return;      
    }
    pos_type s_off = 64 - pos % 64;
    pos_type s_idx = pos / 64;
    SetValPos(pos, 0, s_off, is_on_data);

    width -= s_off;
    s_idx += 1;
    while (width > 64){
      if (is_on_data)
        data_[s_idx] = 0;
      else
        flag_[s_idx] = 0;
      width -= 64;
      s_idx += 1;
    }
    SetValPos(s_idx * 64, 0, width, is_on_data);  
  }


  inline void BulkCopy_forward(pos_type from, pos_type destination, width_type bits, bool is_on_data)
  {
    while (bits > 64){
      SetValPos(destination, GetValPos(from, 64, is_on_data), 64, is_on_data);
      from += 64;
      destination += 64;
      bits -= 64;
    }
    SetValPos(destination, GetValPos(from, bits, is_on_data), bits, is_on_data);
  }

  // from position is one bit to the right
  inline void BulkCopy_backward(pos_type from, pos_type destination, width_type bits, bool is_on_data)
  {
    while (bits > 64){
      SetValPos(destination - 64, GetValPos(from - 64, 64, is_on_data), 64, is_on_data);
      bits -= 64;
      destination -= 64;
      from -= 64;
    }
    SetValPos(destination - bits, GetValPos(from - bits, bits, is_on_data), bits, is_on_data);
  }

  inline void shift_backward(preorder_t from_node, size_type num_nodes)
  {
    width_type shift_amount = dimension_ * num_nodes;
   
    size_type orig_data_size = data_size_;
    size_type orig_flag_size = flag_size_;
    
    increase_bits(shift_amount, true);
    increase_bits(num_nodes, false);

    pos_type start_node_pos = get_node_data_pos(from_node);

    BulkCopy_backward(orig_data_size, data_size_,  orig_data_size - start_node_pos, true);
    BulkCopy_backward(orig_flag_size, flag_size_, orig_flag_size - from_node, false);

    ClearWidth(start_node_pos, shift_amount, true);
    ClearWidth(from_node, num_nodes, false);
  }

  inline void shift_backward_to_uncollapse(preorder_t from_node)
  {
    if (!is_collapse(from_node)){
      return;
    }
    size_t orig_data_size = data_size_;
    width_type shift_amount = num_branches_ - dimension_;
    increase_bits(shift_amount, true);  

    pos_type from_node_pos = get_node_data_pos(from_node);
    BulkCopy_backward(orig_data_size, orig_data_size + shift_amount,  orig_data_size - from_node_pos, true);

    ClearWidth(from_node_pos, num_branches_, true);
    SETBITVAL(flag_, from_node);
  }

  inline void clear_node(preorder_t node){
    pos_type node_pos = get_node_data_pos(node);
    ClearWidth(node_pos, dimension_, true);
    ClearWidth(node, 1, false);
  }

  inline void bulk_clear_node(preorder_t start_node, preorder_t end_node)
  {
    pos_type start_node_pos = get_node_data_pos(start_node);
    pos_type end_node_pos = get_node_data_pos(end_node + 1);

    ClearWidth(start_node_pos, end_node_pos - start_node_pos, true);
    ClearWidth(start_node, end_node + 1 - start_node, false);
  }

  inline void shift_forward_to_collapse(preorder_t from_node)
  {
    if (is_collapse(from_node)){
      return;
    }
    width_type shift_amount = num_branches_ - dimension_;

    pos_type from_node_next_pos = get_node_data_pos(from_node + 1);
    BulkCopy_forward(from_node_next_pos, from_node_next_pos - shift_amount, data_size_ - from_node_next_pos, true);

    decrease_bits(shift_amount, true);  
    ClearWidth(from_node_next_pos - dimension_, dimension_, true);
    CLRBITVAL(flag_, from_node);
  }

  inline void shift_forward(preorder_t from_node, preorder_t to_node)
  {

    if (from_node >= flag_size_){
      raise(SIGINT);
    }
    pos_type from_node_pos = get_node_data_pos(from_node);
    pos_type to_node_pos = get_node_data_pos(to_node);
    BulkCopy_forward(from_node_pos, to_node_pos, data_size_ - from_node_pos, true);
    BulkCopy_forward(from_node, to_node, flag_size_ - from_node, false);
    pos_type shifted_amount = from_node_pos - to_node_pos;
    ClearWidth(data_size_ - shifted_amount, shifted_amount, true);
    ClearWidth(flag_size_ - (from_node - to_node), (from_node - to_node), false);

    // ClearWidth(start_node_pos, end_node_pos - start_node_pos, true);
    // ClearWidth(start_node, end_node + 1 - start_node, false);
    // preorder_t amount_shifted = from_node - to_node;
    // bulk_clear_node(flag_size_ - amount_shifted, flag_size_ - 1);
  }

  inline bool is_collapse(preorder_t node){
    return !GETBITVAL(flag_, node);
  }

  inline bool has_symbol(preorder_t node, symbol_t symbol){

    if (node >= flag_size_){
      return false;
    }
    pos_type node_pos = get_node_data_pos(node);
    if (is_collapse(node)){
      return symbol == GetValPos(node_pos, dimension_, true);
    }
    else {
      return GETBITVAL(data_, node_pos + symbol);
    }
  }

  inline preorder_t get_n_children_from_node_pos(node_t node, pos_type node_pos)
  {
    if (is_collapse(node)){
      return 1;
    }
    else {
      return popcount(node_pos, num_branches_, true);
    }
  }

  inline preorder_t get_n_children(node_t node)
  {
    if (is_collapse(node)){
      return 1;
    }
    else {
      return popcount(get_node_data_pos(node), num_branches_, true);
    }
  }

  inline preorder_t get_child_skip(node_t node, symbol_t symbol) 
  {
    pos_type node_pos = get_node_data_pos(node);
    if (is_collapse(node)){
      symbol_t only_symbol = GetValPos(node_pos, dimension_, true);
      if (symbol > only_symbol)
        return 1;
      return 0;
    }
    else {
      return popcount(node_pos, symbol, true);
    }  
  }

  unsigned nthset(uint64_t x, unsigned n) {
      return __builtin_ctzll(_pdep_u64(1ULL << n, x));
  }

  symbol_t get_k_th_set_bit(preorder_t node, unsigned k /* 0-indexed */, pos_type node_pos){
    // Assume always present
    if (is_collapse(node)){
      return GetValPos(node_pos, dimension_, true);
    }
    symbol_t pos_left = 1 << dimension_;
    symbol_t return_symbol = 0;

    while (pos_left > 64){
      uint64_t next_block = GetValPos(node_pos + return_symbol, 64, true);
      if (next_block){  
        symbol_t set_bit_count = __builtin_popcountll(next_block);
        if (k >= set_bit_count){
          k -= set_bit_count;
        }
        else if (set_bit_count > 0) {
          return return_symbol + nthset(next_block, k);
        }
      }
      pos_left -= 64;   
      return_symbol += 64; 
    }
    
    uint64_t next_block = GetValPos(node_pos + return_symbol, pos_left, true);
    return return_symbol + nthset(next_block, k);
  }

  inline symbol_t next_symbol_with_node_pos(symbol_t symbol, preorder_t node, symbol_t end_symbol_range, pos_type node_pos){

    if (is_collapse(node)){
      symbol_t only_symbol = GetValPos(node_pos, dimension_, true);
      if (symbol <= only_symbol)
        return only_symbol;
      
      return end_symbol_range + 1;
    }

    symbol_t limit = end_symbol_range - symbol + 1;
    bool over_64 = false;
    if (limit > 64){
        limit = 64;
        over_64 = true;
    }
    uint64_t next_block = GetValPos(node_pos + symbol, limit, true);
    if (next_block){
        return __builtin_ctzll(next_block) + symbol;
    }
    else {
        if (over_64){
            return next_symbol_with_node_pos(symbol + limit, node, end_symbol_range, node_pos);
        }
        return end_symbol_range + 1;
>>>>>>> 56cd4ddcc9bc599ba0cdd283885d208e04a0032b
    }
  }

  // Serialization/De-serialization
  virtual size_type Serialize(std::ostream& out) {
    size_t out_size = 0;

    out.write(reinterpret_cast<const char *>(&size_), sizeof(size_type));
    out_size += sizeof(size_type);

    out.write(reinterpret_cast<const char *>(data_),
              sizeof(data_type) * BITS2BLOCKS(size_));
    out_size += (BITS2BLOCKS(size_) * sizeof(uint64_t));

    return out_size;
  }

  virtual size_type Deserialize(std::istream& in) {
    size_t in_size = 0;

    in.read(reinterpret_cast<char *>(&size_), sizeof(size_type));
    in_size += sizeof(size_type);

    data_ = new data_type[BITS2BLOCKS(size_)];
    in.read(reinterpret_cast<char *>(data_),
    BITS2BLOCKS(size_) * sizeof(data_type));
    in_size += (BITS2BLOCKS(size_) * sizeof(data_type));

    return in_size;
  }

 protected:
  // Data members
  data_type *data_;
  size_type size_;
};

}

#endif