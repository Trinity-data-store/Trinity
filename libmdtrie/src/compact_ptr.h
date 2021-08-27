#ifndef COMPACT_PTR_H
#define COMPACT_PTR_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include "delta_encoded_array.h"
// #include "defs.h"

const uint8_t compact_pointer_vector_size_limit = 20;

namespace bits {

class compact_ptr {
 public:

  compact_ptr(void *ptr, size_t size) : ptr_(((uintptr_t) ptr) >> 4ULL), size_(size) {
    size_ = 1;
  }

  std::vector<uint64_t> *get_vector_pointer(){
    return (std::vector<uint64_t> *) (ptr_ << 4ULL);
  }

  bitmap::EliasGammaDeltaEncodedArray<uint64_t> *get_delta_encoded_array_pointer(){
    return (bitmap::EliasGammaDeltaEncodedArray<uint64_t> *) (ptr_ << 4ULL);
  }

  bool binary_if_present(std::vector<uint64_t> *vect, uint64_t primary_key){
      uint64_t low = 0;
      uint64_t high = vect->size() - 1;

      while (low + 1 < high){
          uint64_t mid = (low + high) / 2;
          if ((*vect)[mid] < primary_key){
              low = mid;
          }
          else {
              high = mid;
          }
      }
      if ((*vect)[low] == primary_key || (*vect)[high] == primary_key){
          return true;
      }
      return false;
  }

  uint64_t size_overhead(){

    if (check_is_vector()){
      std::vector<uint64_t> *vect_ptr = get_vector_pointer();
      return sizeof(*vect_ptr) + (sizeof(uint64_t) * vect_ptr->size());
    }
    else {
      return get_delta_encoded_array_pointer()->size_overhead();
    }
  }

  void push(uint64_t primary_key){
    size_ ++;
    if (size_ == compact_pointer_vector_size_limit + 1){
      // raise(SIGINT);
      std::vector<uint64_t> *vect_ptr = get_vector_pointer();

      auto enc_array = new bitmap::EliasGammaDeltaEncodedArray<uint64_t>(*vect_ptr, vect_ptr->size());
      delete vect_ptr;
      // bitmap::EliasGammaDeltaEncodedArray<uint64_t> enc_array(*vect_ptr, vect_ptr->size());
      ptr_ = ((uintptr_t) enc_array) >> 4ULL;
    }
    if (check_is_vector()){
      get_vector_pointer()->push_back(primary_key);
    }
    else {
      get_delta_encoded_array_pointer()->Push(primary_key);
    }    
  }

  uint64_t get(uint32_t index){
    if (check_is_vector()){
      return (*get_vector_pointer())[index];
    }
    else {
      return (*get_delta_encoded_array_pointer())[index];
    }       

  }

  bool check_if_present(uint64_t primary_key){

    if (check_is_vector()){
      return binary_if_present(get_vector_pointer(), primary_key);
    }
    else {
      return get_delta_encoded_array_pointer()->Find(primary_key);
    } 

  }

  bool check_is_vector(){
    // defined in defs.h
    return size_ <= compact_pointer_vector_size_limit;
  }

  size_t size() const {
    return size_;
  }

 private:
  uintptr_t ptr_: 44;
  size_t size_ : 20;
};

}

#endif // COMPACT_PTR_H