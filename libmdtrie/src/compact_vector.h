#ifndef BITMAP_BITMAP_ARRAY_H_
#define BITMAP_BITMAP_ARRAY_H_

#include "bit_vector.h"

#include <limits>

namespace bitmap {

// Reference to a value
template<typename VectorImpl>
class value_reference {
 public:
  typedef typename VectorImpl::pos_type pos_type;
  typedef typename VectorImpl::value_type value_type;
  typedef typename VectorImpl::reference reference;

  value_reference(VectorImpl *array, pos_type pos)
      : array_(array),
        pos_(pos) {
  }

  reference &operator=(value_type val) {
    array_->Set(pos_, val);
    return *this;
  }

  reference &operator=(const value_reference &ref) {
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

  value_reference &operator--() {
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

  bool operator==(const value_reference &x) const {
    return value_type(*this) == value_type(x);
  }

  bool operator<(const value_reference &x) const {
    return value_type(*this) < value_type(x);
  }

  friend void swap(reference &lhs, reference &rhs) {
    value_type temp = value_type(lhs);
    lhs = rhs;
    rhs = temp;
  }

  friend void swap(reference lhs, reference rhs) {
    value_type temp = value_type(lhs);
    lhs = rhs;
    rhs = temp;
  }

  friend void swap(reference lhs, value_type rhs) {
    value_type temp = value_type(lhs);
    lhs = rhs;
    rhs = temp;
  }

  friend void swap(value_type lhs, reference rhs) {
    value_type temp = value_type(rhs);
    rhs = lhs;
    lhs = temp;
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

  typedef value_reference<CompactVector<T, W>> reference;
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

  void swap(const CompactVector<T, W> &other) {
    using std::swap;
    swap(this->data_, other.data_);
    swap(this->size_, other.size_);
  }

  // Serialization and De-serialization
  size_type Serialize(std::ostream &out) override {
    return BitVector::Serialize(out);
  }

  size_type Deserialize(std::istream &in) override {
    return BitVector::Deserialize(in);
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

  // Accessors, Mutators
  void *At(pos_type idx) {
    return reinterpret_cast<void *>(CompactVector<uint64_t, 44>::Get(idx) << 4ULL);
  }

  void PushBack(void *val) {
    CompactVector<uint64_t, 44>::Append(reinterpret_cast<uint64_t>(val) >> 4ULL);
  }
};

}
#endif