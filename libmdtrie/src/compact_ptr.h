#ifndef COMPACT_PTR_H
#define COMPACT_PTR_H

#include "delta_encoded_array.h"
#include <cstddef>
#include <cstdint>
#include <vector>

const uint64_t compact_pointer_vector_size_limit = 14252681;

namespace bits
{

  class compact_ptr
  {
  public:
    compact_ptr(n_leaves_t primary_key)
    {

      ptr_ = (uintptr_t)primary_key;
      flag_ = 0;
    }

    compact_ptr() {}

    std::vector<n_leaves_t> *get_vector_pointer()
    {
      return (std::vector<n_leaves_t> *)(ptr_ << 4ULL);
    }

    bitmap::EliasGammaDeltaEncodedArray<n_leaves_t> *
    get_delta_encoded_array_pointer()
    {
      return (bitmap::EliasGammaDeltaEncodedArray<n_leaves_t> *)(ptr_ << 4ULL);
    }

    bool scan_if_present(std::vector<n_leaves_t> *vect, n_leaves_t primary_key)
    {
      int vect_size = vect->size();

      for (int i = 0; i < vect_size; i++)
      {
        if ((*vect)[i] == primary_key)
          return true;
      }
      return false;
    }

    bool binary_if_present(std::vector<n_leaves_t> *vect, n_leaves_t primary_key)
    {
      uint64_t low = 0;
      uint64_t high = vect->size() - 1;

      while (low + 1 < high)
      {
        uint64_t mid = (low + high) / 2;
        if ((*vect)[mid] < primary_key)
        {
          low = mid;
        }
        else
        {
          high = mid;
        }
      }
      if ((*vect)[low] == primary_key || (*vect)[high] == primary_key)
      {
        return true;
      }
      return false;
    }

    uint64_t size_overhead()
    {

      if (flag_ == 0)
      {
        return 0 /*sizeof(compact_ptr)*/;
      }
      if (flag_ == 1)
      {
        std::vector<n_leaves_t> *vect_ptr = get_vector_pointer();
        return /*sizeof(*vect_ptr) + */ sizeof(n_leaves_t) /*primary key size*/ *
               vect_ptr->size() /*+ sizeof(compact_ptr)*/;
      }
      else
      {
        return get_delta_encoded_array_pointer()
            ->size_overhead() /*+ sizeof(compact_ptr)*/;
      }
    }

    void push(n_leaves_t primary_key)
    {

      if (flag_ == 0)
      {
        auto array = new std::vector<n_leaves_t>;
        array->push_back((n_leaves_t)ptr_);
        array->push_back(primary_key);
        ptr_ = ((uintptr_t)array) >> 4ULL;
        flag_ = 1;
        return;
      }
      else if (size() == compact_pointer_vector_size_limit + 1)
      {

        std::vector<n_leaves_t> *vect_ptr = get_vector_pointer();

        auto enc_array = new bitmap::EliasGammaDeltaEncodedArray<n_leaves_t>(
            *vect_ptr, vect_ptr->size());
        delete vect_ptr;
        ptr_ = ((uintptr_t)enc_array) >> 4ULL;
        flag_ = 2;
      }
      if (flag_ == 1)
      {
        get_vector_pointer()->push_back(primary_key);
      }
      else
      {
        get_delta_encoded_array_pointer()->Push(primary_key);
      }
    }

    uint64_t get(n_leaves_t index)
    {
      if (index >= size())
        return 0;

      if (flag_ == 0)
      {
        return (uint64_t)ptr_;
      }
      if (flag_ == 1)
      {
        return (*get_vector_pointer())[index];
      }
      else
      {
        return (*get_delta_encoded_array_pointer())[index];
      }
    }

    bool check_if_present(n_leaves_t primary_key)
    {

      if (flag_ == 0)
      {
        return primary_key == (uint64_t)ptr_;
      }
      else if (flag_ == 1)
      {
        // return binary_if_present(get_vector_pointer(), primary_key);
#ifdef USE_LINEAR_SCAN
        return scan_if_present(get_vector_pointer(), primary_key);
#else
        return binary_if_present(get_vector_pointer(), primary_key);
#endif
      }
      else
      {
        return get_delta_encoded_array_pointer()->Find(primary_key);
      }
    }

    size_t size()
    {

      if (flag_ == 0)
      {
        return 1;
      }
      else if (flag_ == 1)
      {
        return get_vector_pointer()->size();
      }
      return get_delta_encoded_array_pointer()->get_num_elements();
    }

  private:
    uintptr_t ptr_ : 44;
    unsigned flag_ : 2;
  } __attribute__((packed));

} // namespace bits

#endif // COMPACT_PTR_H