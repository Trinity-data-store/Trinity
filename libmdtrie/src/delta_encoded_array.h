#ifndef BITMAP_DELTA_ENCODED_ARRAY_H_
#define BITMAP_DELTA_ENCODED_ARRAY_H_

#include <vector>
#include "bitmap.h"
#include "bitmap_array.h"
#include "utils.h"

#define USE_PREFIXSUM_TABLE 1

namespace bitmap {

template<typename T, uint32_t sampling_rate = 32>
class DeltaEncodedArray {
 public:
  typedef size_t size_type;
  typedef size_t pos_type;
  typedef uint8_t width_type;

  DeltaEncodedArray() = default;
  virtual ~DeltaEncodedArray() = default;

  virtual size_type Serialize(std::ostream& out) {
    size_type out_size = 0;
    out_size += samples_.Serialize(out);
    out_size += delta_offsets_.Serialize(out);
    out_size += deltas_.Serialize(out);

    out.write(reinterpret_cast<const char *>(&num_elements_), sizeof(size_type));
    out_size += sizeof(size_type);    

    // T last_val_;
    out.write(reinterpret_cast<const char *>(&last_val_), sizeof(T));
    out_size += sizeof(T);  

    return out_size;
  }

  virtual size_type Deserialize(std::istream& in) {

    size_type in_size = 0;

    in_size += samples_.Deserialize(in);
    in_size += delta_offsets_.Deserialize(in);
    in_size += deltas_.Deserialize(in);

    in.read(reinterpret_cast<char *>(&num_elements_), sizeof(size_type));
    in_size += sizeof(size_type);    

    // T last_val_;
    in.read(reinterpret_cast<char *>(&last_val_), sizeof(T));
    in_size += sizeof(T);  

    return in_size;
  }


 protected:
  // Get the encoding size for an delta value
  virtual width_type EncodingSize(T delta) = 0;

  // Encode the delta values
  virtual void EncodeDeltas(std::vector<T> & deltas, size_type num_deltas) = 0;
  virtual void EncodeLastDelta(T delta, pos_type pos) = 0;

  // Encode the delta encoded array
  void Encode(std::vector<T> & elements, size_type num_elements) {
    if (num_elements == 0) {
      return;
    }
    num_elements_ = num_elements;
    
#ifdef DEBUG
    assert(std::is_sorted(elements, elements + num_elements));
#endif

    std::vector<T> samples, deltas;
    std::vector<pos_type> delta_offsets;
    T last_val = 0;
    size_type cum_delta_size = 0;

    for (size_t i = 0; i < num_elements; i++) {
      if (i % sampling_rate == 0) {

        samples.push_back(elements[i]);
        delta_offsets.push_back(cum_delta_size);
      } else {

        T delta = elements[i] - last_val;
        deltas.push_back(delta);
        cum_delta_size += EncodingSize(delta);
      }
      last_val = elements[i];
    }

    if (samples.size() != 0) {

      samples_.Init(&samples[0], samples.size());
    }
    last_val_ = last_val;
    
    if (cum_delta_size != 0){

      deltas_.Bitmap_Init(cum_delta_size);
      EncodeDeltas(deltas, deltas.size());
    }

    if (delta_offsets.size() != 0){

      delta_offsets_.Init(&delta_offsets[0],delta_offsets.size());
    }
  }
  UnsizedBitmapArray<T> samples_;
  UnsizedBitmapArray<pos_type> delta_offsets_;
  Bitmap deltas_;
  size_type num_elements_ = 0;
  T last_val_;
  private:
};

static struct EliasGammaPrefixSum {
 public:
  typedef uint16_t block_type;

  EliasGammaPrefixSum() {
    for (uint64_t i = 0; i < 65536; i++) {
      uint16_t val = (uint16_t) i;
      uint64_t count = 0, offset = 0, sum = 0;
      while (val && offset <= 16) {
        int N = 0;
        while (!GETBIT(val, offset)) {
          N++;
          offset++;
        }
        offset++;
        if (offset + N <= 16) {
          sum += ((val >> offset) & ((uint16_t) low_bits_set[N])) + (1 << N);
          offset += N;
          count++;
        } else {
          offset -= (N + 1);
          break;
        }
      }
      prefixsum_[i] = (offset << 24) | (count << 16) | sum;
    }
  }

  uint16_t offset(const block_type i) const {
    return ((prefixsum_[(i)] >> 24) & 0xFF);
  }

  uint16_t count(const block_type i) const {
    return ((prefixsum_[i] >> 16) & 0xFF);
  }

  uint16_t sum(const block_type i) const {
    return (prefixsum_[i] & 0xFFFF);
  }
 private:
  uint32_t prefixsum_[65536];
} elias_gamma_prefix_table;

template<typename T, uint32_t sampling_rate = 32>
class EliasGammaDeltaEncodedArray : public DeltaEncodedArray<T, sampling_rate> {
 public:
  typedef typename DeltaEncodedArray<T>::size_type size_type;
  typedef typename DeltaEncodedArray<T>::pos_type pos_type;
  typedef typename DeltaEncodedArray<T>::width_type width_type;

  using DeltaEncodedArray<T>::EncodingSize;
  using DeltaEncodedArray<T>::EncodeDeltas;
  using DeltaEncodedArray<T>::EncodeLastDelta;

  EliasGammaDeltaEncodedArray()
      : DeltaEncodedArray<T>() {
  }

  EliasGammaDeltaEncodedArray(std::vector<T> & elements, size_type num_elements)
      : EliasGammaDeltaEncodedArray<T>() {
    this->Encode(elements, num_elements);
  }

  virtual ~EliasGammaDeltaEncodedArray() {
  }

  size_type GetNElements(){

    return this->num_elements_;
  }

  T Get(pos_type i) {

    pos_type samples_idx = i / sampling_rate;
    pos_type delta_offsets_idx = i % sampling_rate;
    T val = this->samples_.Get(samples_idx);

    if (delta_offsets_idx == 0)
      return val;

    pos_type delta_offset = this->delta_offsets_.Get(samples_idx);
    val += PrefixSum(delta_offset, delta_offsets_idx);
    return val;
  }

  T operator[](pos_type i) {
    return Get(i);
  }

  uint64_t size_overhead() {
    uint64_t size = 0;

    size += this->samples_.GetSize() /*bitmap size*/ /* + sizeof(width_type)*/;
    size += this->delta_offsets_.GetSize() /*bitmap size*/ /* + sizeof(width_type)*/;
    size += this->deltas_.GetSize();
    size += sizeof(T);
    return size;
  }

  void Push(T element){

      if (this->GetNElements() % sampling_rate == 0){
        
          size_type cum_delta_size = this->deltas_.GetSizeInBits();
          this->samples_.Push(element);
          this->delta_offsets_.Push(cum_delta_size);
      }
      else {

          T delta = element - this->last_val_;
          if (this->GetNElements() == 1){
              this->deltas_.Bitmap_Init(EncodingSize(delta));
              EncodeLastDelta(delta, 0);  
          }
          else{
            pos_type pos = this->deltas_.GetSizeInBits();
            this->deltas_.Realloc_increase(EncodingSize(delta));
            EncodeLastDelta(delta, pos);    
          }
      }    
      this->last_val_ = element;
      this->num_elements_ ++;
  }

  pos_type  BinarySearchSample(int64_t val){

    int64_t start = 0;
    int64_t end = (this->GetNElements() - 1) / sampling_rate;
    int64_t mid, mid_val;

    while (start <= end){

      mid = (start + end) / 2;
      mid_val = this->samples_.Get(mid);
      if (mid_val == val){
        return mid;
      }
      else if (val < mid_val){
        end = mid - 1;
      }
      else {
        start = mid + 1;
      }
    }
    return static_cast<pos_type>(std::max(end, INT64_C(0)));
  }

  pos_type LinearSearchSample(uint64_t val){

    int64_t start = 0;
    int64_t end = (this->GetNElements() - 1) / sampling_rate;
    pos_type lower_bound = start;

    while (start <= end){

      if (this->samples_.Get(start) <= val){
        lower_bound = start;
      }
      else {
        return lower_bound;
      }
      start ++;
    }
    return lower_bound;
  }

  bool Find(T val, pos_type *found_idx = nullptr) {

    pos_type sample_off = BinarySearchSample(val);
    pos_type current_delta_offset = this->delta_offsets_.Get(sample_off);
    val -= this->samples_.Get(sample_off);
    
    if (val < 0){
      return false;
    }

    pos_type delta_idx = 0;
    T delta_sum = 0;
    size_type delta_max = this->deltas_.GetSizeInBits();

    while (delta_sum < val && current_delta_offset < delta_max && delta_idx < sampling_rate) {
      uint16_t block = this->deltas_.GetValPos(current_delta_offset, 16);
      uint16_t block_cnt = elias_gamma_prefix_table.count(block);
      uint16_t block_sum = elias_gamma_prefix_table.sum(block);

      if (block_cnt == 0) {
        // If the prefixsum table for the block returns count == 0
        // this must mean the value spans more than 16 bits
        // read this manually
        uint8_t delta_width = 0;
        while (!this->deltas_.GetBit(current_delta_offset)) {
          delta_width++;
          current_delta_offset++;
        }
        current_delta_offset++;
        auto decoded_value = this->deltas_.GetValPos(current_delta_offset, delta_width) + (1ULL << delta_width);
        delta_sum += decoded_value;
        current_delta_offset += delta_width;
        delta_idx += 1;

        // Roll back
        if (delta_idx == sampling_rate) {
          delta_idx--;
          delta_sum -= decoded_value;
          break;
        }
      } else if (delta_sum + block_sum < val) {
        // If sum can be computed from the prefixsum table
        delta_sum += block_sum;
        current_delta_offset += elias_gamma_prefix_table.offset(block);
        delta_idx += block_cnt;
      } else {
        // Last few values, decode them without looking up table
        T last_decoded_value = 0;
        while (delta_sum < val && current_delta_offset < delta_max && delta_idx < sampling_rate) {
          int delta_width = 0;
          while (!this->deltas_.GetBit(current_delta_offset)) {
            delta_width++;
            current_delta_offset++;
          }
          current_delta_offset++;
          last_decoded_value = this->deltas_.GetValPos(current_delta_offset, delta_width) + (1ULL << delta_width);

          delta_sum += last_decoded_value;
          current_delta_offset += delta_width;
          delta_idx += 1;
        }

        // Roll back
        if (delta_idx == sampling_rate) {
          delta_idx--;
          delta_sum -= last_decoded_value;
          break;
        }
      }
    }

    if (found_idx) {
      pos_type res = sample_off * sampling_rate + delta_idx;
      *found_idx = (delta_sum <= val) ? res : res - 1;
    }
    return val == delta_sum;
  }


  bool BinarySearch(T search_val){

    pos_type sample_index = BinarySearchSample(search_val); // Works
    pos_type delta_offset = this->delta_offsets_.Get(sample_index);
    T current_val =  this->samples_.Get(sample_index);

    if (current_val == search_val){
      return true;
    }

    pos_type limit = sampling_rate;

    if (this->GetNElements() - sample_index * sampling_rate < sampling_rate){
      limit = this->GetNElements() - sample_index * sampling_rate;
    }

    T prefix_sum = 0;
    for (pos_type delta_offsets_idx = 1; delta_offsets_idx < limit; delta_offsets_idx++)
    {
      prefix_sum += PrefixSum_cumulative(delta_offset, delta_offsets_idx, delta_offsets_idx - 1);
      
      if (prefix_sum + current_val > search_val){
        return false;
      }
      if (prefix_sum + current_val == search_val){
        return true;
      }
    }

    return false;
  }

  size_type get_num_elements(){
    return this->num_elements_;
  }
  T PrefixSum_cumulative(pos_type delta_offset, pos_type until_idx, pos_type delta_idx) {
    T delta_sum = 0;
    pos_type current_delta_offset = delta_offset;
    while (delta_idx != until_idx) {
      uint16_t block = this->deltas_.GetValPos(current_delta_offset, 16);
      uint16_t cnt = elias_gamma_prefix_table.count(block);
      if (cnt == 0) {
        // If the prefixsum table for the block returns count == 0
        // this must mean the value spans more than 16 bits
        // read this manually
        width_type delta_width = 0;
        while (!this->deltas_.GetBit(current_delta_offset)) {
          delta_width++;
          current_delta_offset++;
        }
        current_delta_offset++;
        delta_sum += this->deltas_.GetValPos(current_delta_offset, delta_width)
            + (1ULL << delta_width);
        current_delta_offset += delta_width;
        delta_idx += 1;
      } else if (delta_idx + cnt <= until_idx) {
        // If sum can be computed from the prefixsum table
        delta_sum += elias_gamma_prefix_table.sum(block);
        current_delta_offset += elias_gamma_prefix_table.offset(block);
        delta_idx += cnt;
      } else {
        // Last few values, decode them without looking up table
        while (delta_idx != until_idx) {
          width_type delta_width = 0;
          while (!this->deltas_.GetBit(current_delta_offset)) {
            delta_width++;
            current_delta_offset++;
          }
          current_delta_offset++;
          delta_sum += this->deltas_.GetValPos(current_delta_offset,
                                                delta_width)
              + (1ULL << delta_width);
          current_delta_offset += delta_width;
          delta_idx += 1;
        }
      }
    }
    return delta_sum;
  }

 private:
  virtual width_type EncodingSize(T delta) override {
    return 2 * (Utils::BitWidth(delta) - 1) + 1;
  }

  virtual void EncodeDeltas(std::vector<T> & deltas, size_type num_deltas) override {
    uint64_t pos = 0;
    for (size_t i = 0; i < num_deltas; i++) {
      uint64_t delta_bits = Utils::BitWidth(deltas[i]) - 1;
      pos += delta_bits;
      assert((1ULL << delta_bits) <= deltas[i]);
      this->deltas_.SetBit(pos++);
      this->deltas_.SetValPos(pos, deltas[i] - (1ULL << delta_bits),
                               delta_bits);
      pos += delta_bits;
    }
  }

  virtual void EncodeLastDelta(T delta, pos_type pos) override{

      uint64_t delta_bits = Utils::BitWidth(delta) - 1;
      pos += delta_bits;
      this->deltas_.SetBit(pos++);
      this->deltas_.SetValPos(pos, delta - (1ULL << delta_bits),
                               delta_bits);
  }


  T PrefixSum(pos_type delta_offset, pos_type until_idx) {
    T delta_sum = 0;
    pos_type delta_idx = 0;
    pos_type current_delta_offset = delta_offset;
    while (delta_idx != until_idx) {
      uint16_t block = this->deltas_.GetValPos(current_delta_offset, 16);
      uint16_t cnt = elias_gamma_prefix_table.count(block);
      if (cnt == 0) {
        // If the prefixsum table for the block returns count == 0
        // this must mean the value spans more than 16 bits
        // read this manually
        width_type delta_width = 0;
        while (!this->deltas_.GetBit(current_delta_offset)) {
          delta_width++;
          current_delta_offset++;
        }
        current_delta_offset++;
        delta_sum += this->deltas_.GetValPos(current_delta_offset, delta_width)
            + (1ULL << delta_width);
        current_delta_offset += delta_width;
        delta_idx += 1;
      } else if (delta_idx + cnt <= until_idx) {
        // If sum can be computed from the prefixsum table
        delta_sum += elias_gamma_prefix_table.sum(block);
        current_delta_offset += elias_gamma_prefix_table.offset(block);
        delta_idx += cnt;
      } else {
        // Last few values, decode them without looking up table
        while (delta_idx != until_idx) {
          width_type delta_width = 0;
          while (!this->deltas_.GetBit(current_delta_offset)) {
            delta_width++;
            current_delta_offset++;
          }
          current_delta_offset++;
          delta_sum += this->deltas_.GetValPos(current_delta_offset,
                                                delta_width)
              + (1ULL << delta_width);
          current_delta_offset += delta_width;
          delta_idx += 1;
        }
      }
    }
    return delta_sum;
  }
};
}

#endif // BITMAP_DELTA_ENCODED_ARRAY_H_