#ifndef COMPACT_PTR_H
#define COMPACT_PTR_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include "delta_encoded_array.h"
// #include "defs.h"

const uint64_t compact_pointer_vector_size_limit = 1000;

namespace bits {

class compact_ptr {
 public:

  // compact_ptr(void *ptr, size_t size) : ptr_(((uintptr_t) ptr) >> 4ULL), size_(size) {
  //   // raise(SIGINT);
  //   // size_ = 1;
  //   flag_ = 0;
  // }

  compact_ptr(uint64_t primary_key){

    ptr_ = (uintptr_t) primary_key;
    // size_ = 1;
    flag_ = 0;
    // if ((uint64_t)ptr_ != primary_key){
    //   raise(SIGINT);
    // }
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

    if (flag_ == 0){
      return 0 /*sizeof(compact_ptr)*/;
    }
    if (flag_ == 1){
      std::vector<uint64_t> *vect_ptr = get_vector_pointer();
      return /*sizeof(*vect_ptr) + */ sizeof(uint32_t) /*primary key size*/ * vect_ptr->size() /*+ sizeof(compact_ptr)*/;
    }
    else {
      return get_delta_encoded_array_pointer()->size_overhead() /*+ sizeof(compact_ptr)*/;
    }
  }

  void push(uint64_t primary_key){
    // size_ ++;
    if (flag_ == 0){
        auto array = new std::vector<uint64_t>;
        array->push_back((uint64_t) ptr_);
        array->push_back(primary_key);
        ptr_ = ((uintptr_t) array) >> 4ULL;
        flag_ = 1;
        return;      
    }
    else if (size() == compact_pointer_vector_size_limit + 1){
      // raise(SIGINT);
      std::vector<uint64_t> *vect_ptr = get_vector_pointer();

      auto enc_array = new bitmap::EliasGammaDeltaEncodedArray<uint64_t>(*vect_ptr, vect_ptr->size());
      delete vect_ptr;
      ptr_ = ((uintptr_t) enc_array) >> 4ULL;
      flag_ = 2;
    }
    if (flag_ == 1){
      get_vector_pointer()->push_back(primary_key);
    }
    else {
      // raise(SIGINT);
      get_delta_encoded_array_pointer()->Push(primary_key);
      // if (get(size_ - 1) != primary_key){
      //   raise(SIGINT);
      // }
    }    
  }

  uint64_t get(uint32_t index){
    if (flag_ == 0){
      return (uint64_t)ptr_;
    }
    if (flag_ == 1){
      // if (index >= get_vector_pointer()->size()){
      //   raise(SIGINT);
      // }
      return (*get_vector_pointer())[index];
    }
    else {
      // raise(SIGINT);
      return (*get_delta_encoded_array_pointer())[index];
    }       
  }

  bool check_if_present(uint64_t primary_key){

    if (flag_ == 0){
      return primary_key == (uint64_t)ptr_;
    }
    if (flag_ == 1){
      return binary_if_present(get_vector_pointer(), primary_key);
    }
    else {
      // raise(SIGINT);
      // return get_delta_encoded_array_pointer()->BinarySearch(primary_key);
      
      // size_t found_index = 0;
      return get_delta_encoded_array_pointer()->Find(primary_key);
      // if (found && (* get_delta_encoded_array_pointer())[found_index] != primary_key){
      //   raise(SIGINT);
      //   found = get_delta_encoded_array_pointer()->Find(primary_key, &found_index);
      // }

      // for (size_t i = 0; i < size_; i++){
      //   if ((* get_delta_encoded_array_pointer())[i] == primary_key){
      //     if (!found){
      //       raise(SIGINT);
      //       found = get_delta_encoded_array_pointer()->Find(primary_key);
      //     }
      //     return true;
      //   }
      // }
      // return false;
      // return found;
    } 

  }

  // bool check_is_vector(){

  //   return 
  //   // return size() <= compact_pointer_vector_size_limit /*&& size_ != 1*/;
  // }

  size_t size() {
    if (flag_ == 0){
      return 1;
    }
    if (flag_ == 1){
      return get_vector_pointer()->size();
    }
    return get_delta_encoded_array_pointer()->get_num_elements();
  }

 private:
  uintptr_t ptr_: 44;
  // size_t size_ : 20;
  size_t flag_ : 2;
};

}

#endif // COMPACT_PTR_H