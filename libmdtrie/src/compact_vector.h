#ifndef BITMAP_COMPACT_VECTOR_H_
#define BITMAP_COMPACT_VECTOR_H_

#include "bit_vector.h"
#include "delta_encoded_array.h"

#include <limits>

namespace bitmap {

// Reference to a value
template<typename VectorImpl>
class value_reference_vector {
 public:
  typedef typename VectorImpl::pos_type pos_type;
  typedef typename VectorImpl::value_type value_type;
  typedef typename VectorImpl::reference reference;

  value_reference_vector(VectorImpl *array, pos_type pos)
      : array_(array),
        pos_(pos) {
  }

  reference &operator=(value_type val) {
    array_->Set(pos_, val);
    return *this;
  }

  reference &operator=(const value_reference_vector &ref) {
    return (*this) = value_type(ref);
  }

  operator value_type() const {
    return array_->Get(pos_);
  }

  reference &operator++() {
    value_type val = array_->Get(pos_);
    array_->Set(pos_, val + 1);
    return *this;
  }

  value_type operator++(int) {
    auto val = (value_type) *this;
    ++(*this);
    return val;
  }

  value_reference_vector &operator--() {
    value_type val = array_->Get(pos_);
    array_->Set(pos_, val - 1);
    return *this;
  }

  value_type operator--(int) {
    auto val = (value_type) *this;
    --(*this);
    return val;
  }

  reference &operator+=(const value_type x) {
    value_type val = array_->Get(pos_);
    array_->Set(pos_, val + 1);
    return *this;
  }

  reference &operator-=(const value_type x) {
    value_type val = array_->Get(pos_);
    array_->Set(pos_, val - 1);
    return *this;
  }

  bool operator==(const value_reference_vector &x) const {
    return value_type(*this) == value_type(x);
  }

  bool operator<(const value_reference_vector &x) const {
    return value_type(*this) < value_type(x);
  }

 private:
  VectorImpl *const array_;
  const pos_type pos_;
};

// Iterators
template<typename VectorImpl>
class vector_iterator {
 public:
  typedef typename VectorImpl::pos_type pos_type;

  typedef typename VectorImpl::difference_type difference_type;
  typedef typename VectorImpl::value_type value_type;
  typedef typename VectorImpl::pointer pointer;
  typedef typename VectorImpl::reference reference;
  typedef typename VectorImpl::iterator_category iterator_category;

  vector_iterator() {
    array_ = NULL;
    pos_ = 0;
  }

  vector_iterator(VectorImpl *array, pos_type pos) {
    array_ = array;
    pos_ = pos;
  }

  reference operator*() const {
    return reference(array_, pos_);
  }

  vector_iterator &operator++() {
    pos_++;
    return *this;
  }

  vector_iterator operator++(int) {
    vector_iterator it = *this;
    ++(*this);
    return it;
  }

  vector_iterator &operator--() {
    pos_--;
    return *this;
  }

  vector_iterator operator--(int) {
    vector_iterator it = *this;
    --(*this);
    return it;
  }

  vector_iterator &operator+=(difference_type i) {
    pos_ += i;
    return *this;
  }

  vector_iterator &operator-=(difference_type i) {
    pos_ -= i;
    return *this;
  }

  vector_iterator &operator=(const vector_iterator &it) {
    if (this != &it) {
      array_ = it.array_;
      pos_ = it.pos_;
    }
    return *this;
  }

  vector_iterator operator+(difference_type i) const {
    vector_iterator it = *this;
    return it += i;
  }

  vector_iterator operator-(difference_type i) const {
    vector_iterator it = *this;
    return it -= i;
  }

  reference operator[](difference_type i) const {
    return *(*this + i);
  }

  bool operator==(const vector_iterator &it) const {
    return it.pos_ == pos_;
  }

  bool operator!=(const vector_iterator &it) const {
    return *this != it;
  }

  bool operator<(const vector_iterator &it) const {
    return pos_ < it.pos_;
  }

  bool operator>(const vector_iterator &it) const {
    return pos_ > it.pos_;
  }

  bool operator>=(const vector_iterator &it) const {
    return *this >= it;
  }

  bool operator<=(const vector_iterator &it) const {
    return *this <= it;
  }

  difference_type operator-(const vector_iterator &it) {
    return pos_ - it.pos_;
  }

 private:
  VectorImpl *array_;
  pos_type pos_;
};

template<typename VectorImpl>
class const_vector_iterator {
 public:
  typedef typename VectorImpl::pos_type pos_type;

  typedef typename VectorImpl::difference_type difference_type;
  typedef typename VectorImpl::value_type value_type;
  typedef typename VectorImpl::pointer pointer;
  typedef typename VectorImpl::reference reference;
  typedef typename VectorImpl::iterator_category iterator_category;

  typedef typename VectorImpl::value_type const_reference;

  const_vector_iterator(const VectorImpl *array,
                        pos_type pos) {
    array_ = array;
    pos_ = pos;
  }

  const_reference operator*() const {
    return array_->Get(pos_);
  }

  const_vector_iterator &operator++() {
    pos_++;
    return *this;
  }

  const_vector_iterator operator++(int) {
    const_vector_iterator it = *this;
    ++(*this);
    return it;
  }

  const_vector_iterator &operator--() {
    pos_--;
    return *this;
  }

  const_vector_iterator operator--(int) {
    const_vector_iterator it = *this;
    --(*this);
    return it;
  }

  const_vector_iterator &operator+=(difference_type i) {
    pos_ += i;
    return *this;
  }

  const_vector_iterator &operator-=(difference_type i) {
    pos_ -= i;
    return *this;
  }

  const_vector_iterator operator+(difference_type i) const {
    const_vector_iterator it = *this;
    return it += i;
  }

  const_vector_iterator operator-(difference_type i) const {
    const_vector_iterator it = *this;
    return it -= i;
  }

  const_reference operator[](difference_type i) const {
    return *(*this + i);
  }

  bool operator==(const const_vector_iterator &it) const {
    return it.pos_ == pos_;
  }

  bool operator!=(const const_vector_iterator &it) const {
    return *this != it;
  }

  bool operator<(const const_vector_iterator &it) const {
    return pos_ < it.pos_;
  }

  bool operator>(const const_vector_iterator &it) const {
    return pos_ > it.pos_;
  }

  bool operator>=(const const_vector_iterator &it) const {
    return *this >= it;
  }

  bool operator<=(const const_vector_iterator &it) const {
    return *this <= it;
  }

  difference_type operator-(const const_vector_iterator &it) {
    return pos_ - it.pos_;
  }

 private:
  const VectorImpl *array_;
  pos_type pos_;
};

template<typename T, uint8_t W>
class CompactVector : public BitVector {
 public:
  static_assert(!std::numeric_limits<T>::is_signed, "Signed types cannot be used.");
  // Type definitions
  typedef typename BitVector::size_type size_type;
  typedef typename BitVector::width_type width_type;
  typedef typename BitVector::pos_type pos_type;
  typedef int64_t tmp_pos_type;

  typedef value_reference_vector<CompactVector<T, W>> reference;
  typedef T value_type;
  typedef ptrdiff_t difference_type;
  typedef T *pointer;
  typedef vector_iterator<CompactVector<T, W>> iterator;
  typedef const_vector_iterator<CompactVector<T, W>> const_iterator;
  typedef std::random_access_iterator_tag iterator_category;

  // Constructors and destructors
  CompactVector() : BitVector() {}

  CompactVector(const CompactVector &vec) {
    data_ = vec.data_;
    size_ = vec.size_;
  }

  explicit CompactVector(size_type num_elements) : BitVector(num_elements * W) {}

  ~CompactVector() override = default;

  CompactVector(T *elements, size_type num_elements) : CompactVector(num_elements) {
    for (uint64_t i = 0; i < num_elements; i++) {
      Set(i, elements[i]);
    }
  }

  void Init(T *elements, size_type num_elements) {
    BitVector::Init(num_elements * W);
    for (uint64_t i = 0; i < num_elements; i++) {
      Set(i, elements[i]);
    }
  }

  width_type GetBitWidth() const {
    return W;
  }

  size_type size() const {
    return size_ / W;
  }

  bool empty() const {
    return size_ == 0;
  }

  // Accessors and mutators
  void Append(T val) {
    this->AppendVal(val, W);
  }

  void Set(pos_type i, T value) {
    this->SetValPos(i * W, value, W);
  }

  T Get(pos_type i) const {
    return (T) this->GetValPos(i * W, W);
  }

  pos_type LowerBound(T val) const {
    tmp_pos_type sp = 0, ep = size();
    pos_type m;
    while (sp <= ep) {
      m = (sp + ep) / 2;
      T cur = Get(m);
      if (cur == val)
        return m;
      else if (val < cur)
        ep = m - 1;
      else
        sp = m + 1;
    }
    return static_cast<pos_type>(std::max(ep, INT64_C(0)));
  }

  // Operators, iterators
  T operator[](const pos_type &i) const {
    return Get(i);
  }

  reference operator[](const pos_type &i) {
    return reference(this, i);
  }

  iterator begin() {
    return iterator(this, 0);
  }

  const_iterator begin() const {
    return const_iterator(this, 0);
  }

  const_iterator cbegin() const {
    return const_iterator(this, 0);
  }

  iterator end() {
    return iterator(this, this->num_elements_);
  }

  const_iterator end() const {
    return const_iterator(this, this->num_elements_);
  }

  const_iterator cend() const {
    return const_iterator(this, this->num_elements_);
  }
};

class CompactPtrVector : CompactVector<uint64_t, 44> {
 public:

  // Type definitions
  typedef typename BitVector::size_type size_type;
  typedef typename BitVector::width_type width_type;
  typedef typename BitVector::pos_type pos_type;
  typedef int64_t tmp_pos_type;

  CompactPtrVector() : CompactVector<uint64_t, 44>() {}
  CompactPtrVector(size_type num_elements) : CompactVector<uint64_t, 44>(num_elements) {
    num_elements_ = num_elements;
  }

  // Accessors, Mutators
  void *At(pos_type idx) {
    return reinterpret_cast<void *>(CompactVector<uint64_t, 44>::Get(idx) << 4ULL);
  }

  void Set(pos_type idx, void *val) {
    CompactVector<uint64_t, 44>::Set(idx, reinterpret_cast<uint64_t>(val) >> 4ULL);
  }

  void PushBack(void *val) {
    CompactVector<uint64_t, 44>::Append(reinterpret_cast<uint64_t>(val) >> 4ULL);
  }

  uint64_t size_overhead(){
    return CompactVector<uint64_t, 44>::size_overhead();
  }

  size_type get_num_elements(){
    return num_elements_;
  }
  
 private:
  size_type num_elements_;
};

class CompactPrimaryVector : CompactVector<uint64_t, 46> {
 public:

  // Type definitions
  typedef typename BitVector::size_type size_type;
  typedef typename BitVector::width_type width_type;
  typedef typename BitVector::pos_type pos_type;
  typedef int64_t tmp_pos_type;
  const uint64_t compact_pointer_vector_size_limit = 1000;

  CompactPrimaryVector() : CompactVector<uint64_t, 46>() {}
  CompactPrimaryVector(size_type num_elements) : CompactVector<uint64_t, 46>(num_elements) {}

  // Accessors, Mutators
  uint64_t At(pos_type idx) {
    return reinterpret_cast<uint64_t>(CompactVector<uint64_t, 46>::Get(idx) << 20ULL);
  }

  void Set(pos_type idx, uint64_t val) {
    CompactVector<uint64_t, 46>::Set(idx, reinterpret_cast<uint64_t>(val) >> 20ULL);
  }

  void PushBack(uint64_t val) {
    CompactVector<uint64_t, 46>::Append(reinterpret_cast<uint64_t>(val) >> 20ULL);
  }

  uintptr_t ptr(pos_type idx){
    return At(idx) >> 0b11;
  }

  size_t flag(pos_type idx){
    return At(idx) & 0b11;
  }
  
  std::vector<uint64_t> *get_vector_pointer(pos_type idx){

    return (std::vector<uint64_t> *) (ptr(idx) << 4ULL);
  
  }

  bitmap::EliasGammaDeltaEncodedArray<uint64_t> *get_delta_encoded_array_pointer(pos_type idx){
    return (bitmap::EliasGammaDeltaEncodedArray<uint64_t> *) (ptr(idx) << 4ULL);
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

  uint64_t size_overhead(pos_type idx){

    if (flag(idx) == 0){
      return 0 /*sizeof(compact_ptr)*/;
    }
    else if (flag(idx) == 1){
      std::vector<uint64_t> *vect_ptr = get_vector_pointer(idx);
      return /*sizeof(*vect_ptr) + */ sizeof(uint32_t) /*primary key size*/ * vect_ptr->size() /*+ sizeof(compact_ptr)*/;
    }
    else {
      return get_delta_encoded_array_pointer(idx)->size_overhead() /*+ sizeof(compact_ptr)*/;
    }
  }  

  void push(pos_type idx, uint64_t primary_key){

    if (flag(idx) == 0){
        auto array = new std::vector<uint64_t>;
        array->push_back((uint64_t) ptr(idx));
        array->push_back(primary_key);
        set_ptr(idx, ((uintptr_t) array) >> 4ULL);
        set_flag(idx, 1);
        return;      
    }
    else if (size(idx) == compact_pointer_vector_size_limit + 1){

      std::vector<uint64_t> *vect_ptr = get_vector_pointer(idx);

      auto enc_array = new bitmap::EliasGammaDeltaEncodedArray<uint64_t>(*vect_ptr, vect_ptr->size());
      delete vect_ptr;
      set_ptr(idx, ((uintptr_t) enc_array) >> 4ULL);
      set_flag(idx, 2);
    }
    if (flag(idx) == 1){
      get_vector_pointer(idx)->push_back(primary_key);
    }
    else {
      get_delta_encoded_array_pointer(idx)->Push(primary_key);
    }    
  }  

  uint64_t get(pos_type idx, uint32_t index){

    if (flag(idx) == 0){
      return (uint64_t)ptr(idx);
    }
    else if (flag(idx) == 1){
      return (*get_vector_pointer(idx))[index];
    }
    else {
      return (*get_delta_encoded_array_pointer(idx))[index];
    }       
  }

  bool check_if_present(pos_type idx, uint64_t primary_key){

    if (flag(idx) == 0){
      return primary_key == (uint64_t)ptr(idx);
    }
    else if (flag(idx) == 1){
      return binary_if_present(get_vector_pointer(idx), primary_key);

    }
    else {
      return get_delta_encoded_array_pointer(idx)->Find(primary_key);
    } 
  }   

  size_t size(pos_type idx) {

    if (flag(idx) == 0){
      return 1;
    }
    else if (flag(idx) == 1){
      return get_vector_pointer(idx)->size();
    }
    return get_delta_encoded_array_pointer(idx)->get_num_elements();
  }   

  void set_ptr(pos_type idx, uintptr_t ptr){
    Set(idx, At(idx) & (ptr << 2));
  }

  void set_flag(pos_type idx, unsigned flag){
    Set(idx, At(idx) & flag);
  }
};

}
#endif