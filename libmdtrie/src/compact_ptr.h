#ifndef COMPACT_PTR_H
#define COMPACT_PTR_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include "delta_encoded_array.h"
// #include "defs.h"

namespace bits {

class compact_ptr {
 public:

  compact_ptr(void *ptr, size_t size) : ptr_(((uintptr_t) ptr) >> 4ULL), size_(size) {}

  std::vector<uint64_t> *get_vector_pointer(){
    return (std::vector<uint64_t> *) (ptr_ << 4ULL);
  }

  bitmap::EliasGammaDeltaEncodedArray<uint64_t> *get_delta_encoded_array_pointer(){
    return (bitmap::EliasGammaDeltaEncodedArray<uint64_t> *) (ptr_ << 4ULL);
  }

  bool check_is_vector(){
    // TODO: hard-coded
    return size_ <= 20;
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