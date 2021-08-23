#ifndef BITMAP_BITMAP_ARRAY_H_
#define BITMAP_BITMAP_ARRAY_H_

#include "bitmap.h"

#include <limits>

namespace bitmap {

// Reference to a value
template<typename BitmapArrayImplementation>
class value_reference {
 public:
  typedef typename BitmapArrayImplementation::pos_type pos_type;
  typedef typename BitmapArrayImplementation::value_type value_type;
  typedef typename BitmapArrayImplementation::reference reference;

  value_reference(BitmapArrayImplementation* array, pos_type pos)
      : array_(array),
        pos_(pos) {
  }

  reference& operator=(value_type val) {
    array_->Set(pos_, val);
    return *this;
  }

  reference& operator=(const value_reference& ref) {
    return (*this) = value_type(ref);
  }

  operator value_type() const {
    return array_->Get(pos_);
  }

  reference& operator++() {
    value_type val = array_->Get(pos_);
    array_->Set(pos_, val + 1);
    return *this;
  }

  value_type operator++(int) {
    value_type val = (value_type) *this;
    ++(*this);
    return val;
  }

  value_reference& operator--() {
    value_type val = array_->Get(pos_);
    array_->Set(pos_, val - 1);
    return *this;
  }

  value_type operator--(int) {
    value_type val = (value_type) *this;
    --(*this);
    return val;
  }

  reference& operator+=(const value_type x) {
    value_type val = array_->Get(pos_);
    array_->Set(pos_, val + 1);
    return *this;
  }

  reference& operator-=(const value_type x) {
    value_type val = array_->Get(pos_);
    array_->Set(pos_, val - 1);
    return *this;
  }

  bool operator==(const value_reference& x) const {
    return value_type(*this) == value_type(x);
  }

  bool operator<(const value_reference& x) const {
    return value_type(*this) < value_type(x);
  }

  friend void swap(reference& lhs, reference& rhs) {
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
  BitmapArrayImplementation* const array_;
  const pos_type pos_;
};

// Iterators
template<typename BitmapArrayImplementation>
class bitmap_array_iterator {
 public:
  typedef typename BitmapArrayImplementation::pos_type pos_type;

  typedef typename BitmapArrayImplementation::difference_type difference_type;
  typedef typename BitmapArrayImplementation::value_type value_type;
  typedef typename BitmapArrayImplementation::pointer pointer;
  typedef typename BitmapArrayImplementation::reference reference;
  typedef typename BitmapArrayImplementation::iterator_category iterator_category;

  bitmap_array_iterator() {
    array_ = NULL;
    pos_ = 0;
  }

  bitmap_array_iterator(BitmapArrayImplementation* array, pos_type pos) {
    array_ = array;
    pos_ = pos;
  }

  reference operator*() const {
    return reference(array_, pos_);
  }

  bitmap_array_iterator& operator++() {
    pos_++;
    return *this;
  }

  bitmap_array_iterator operator++(int) {
    bitmap_array_iterator it = *this;
    ++(*this);
    return it;
  }

  bitmap_array_iterator& operator--() {
    pos_--;
    return *this;
  }

  bitmap_array_iterator operator--(int) {
    bitmap_array_iterator it = *this;
    --(*this);
    return it;
  }

  bitmap_array_iterator& operator+=(difference_type i) {
    pos_ += i;
    return *this;
  }

  bitmap_array_iterator& operator-=(difference_type i) {
    pos_ -= i;
    return *this;
  }

  bitmap_array_iterator& operator=(const bitmap_array_iterator& it) {
    if (this != &it) {
      array_ = it.array_;
      pos_ = it.pos_;
    }
    return *this;
  }

  bitmap_array_iterator operator+(difference_type i) const {
    bitmap_array_iterator it = *this;
    return it += i;
  }

  bitmap_array_iterator operator-(difference_type i) const {
    bitmap_array_iterator it = *this;
    return it -= i;
  }

  reference operator[](difference_type i) const {
    return *(*this + i);
  }

  bool operator==(const bitmap_array_iterator& it) const {
    return it.pos_ == pos_;
  }

  bool operator!=(const bitmap_array_iterator& it) const {
    return !(*this == it);
  }

  bool operator<(const bitmap_array_iterator& it) const {
    return pos_ < it.pos_;
  }

  bool operator>(const bitmap_array_iterator& it) const {
    return pos_ > it.pos_;
  }

  bool operator>=(const bitmap_array_iterator& it) const {
    return !(*this < it);
  }

  bool operator<=(const bitmap_array_iterator& it) const {
    return !(*this > it);
  }

  difference_type operator-(const bitmap_array_iterator& it) {
    return pos_ - it.pos_;
  }

 private:
  BitmapArrayImplementation *array_;
  pos_type pos_;
};

template<typename BitmapArrayImplementation>
class const_bitmap_array_iterator {
 public:
  typedef typename BitmapArrayImplementation::pos_type pos_type;

  typedef typename BitmapArrayImplementation::difference_type difference_type;
  typedef typename BitmapArrayImplementation::value_type value_type;
  typedef typename BitmapArrayImplementation::pointer pointer;
  typedef typename BitmapArrayImplementation::reference reference;
  typedef typename BitmapArrayImplementation::iterator_category iterator_category;

  typedef typename BitmapArrayImplementation::value_type const_reference;

  const_bitmap_array_iterator(const BitmapArrayImplementation* array,
                              pos_type pos) {
    array_ = array;
    pos_ = pos;
  }

  const_reference operator*() const {
    return array_->Get(pos_);
  }

  const_bitmap_array_iterator& operator++() {
    pos_++;
    return *this;
  }

  const_bitmap_array_iterator operator++(int) {
    const_bitmap_array_iterator it = *this;
    ++(*this);
    return it;
  }

  const_bitmap_array_iterator& operator--() {
    pos_--;
    return *this;
  }

  const_bitmap_array_iterator operator--(int) {
    const_bitmap_array_iterator it = *this;
    --(*this);
    return it;
  }

  const_bitmap_array_iterator& operator+=(difference_type i) {
    pos_ += i;
    return *this;
  }

  const_bitmap_array_iterator& operator-=(difference_type i) {
    pos_ -= i;
    return *this;
  }

  const_bitmap_array_iterator operator+(difference_type i) const {
    const_bitmap_array_iterator it = *this;
    return it += i;
  }

  const_bitmap_array_iterator operator-(difference_type i) const {
    const_bitmap_array_iterator it = *this;
    return it -= i;
  }

  const_reference operator[](difference_type i) const {
    return *(*this + i);
  }

  bool operator==(const const_bitmap_array_iterator& it) const {
    return it.pos_ == pos_;
  }

  bool operator!=(const const_bitmap_array_iterator& it) const {
    return !(*this == it);
  }

  bool operator<(const const_bitmap_array_iterator& it) const {
    return pos_ < it.pos_;
  }

  bool operator>(const const_bitmap_array_iterator& it) const {
    return pos_ > it.pos_;
  }

  bool operator>=(const const_bitmap_array_iterator& it) const {
    return !(*this < it);
  }

  bool operator<=(const const_bitmap_array_iterator& it) const {
    return !(*this > it);
  }

  difference_type operator-(const const_bitmap_array_iterator& it) {
    return pos_ - it.pos_;
  }

 private:
  const BitmapArrayImplementation* array_;
  pos_type pos_;
};

template<typename T>
class BitmapArray : public Bitmap {
 public:
  // Constructors and destructors
  BitmapArray()
      : Bitmap() {
    num_elements_ = 0;
    bit_width_ = 0;
  }

  BitmapArray(const BitmapArray& array) {
    data_ = array.data_;
    size_ = array.size_;
    num_elements_ = array.num_elements_;
    bit_width_ = array.bit_width_;
  }

  BitmapArray(size_type num_elements, width_type bit_width)
      : Bitmap(num_elements * bit_width) {
    num_elements_ = num_elements;
    bit_width_ = bit_width;
  }

  virtual ~BitmapArray() {
  }

  // Getters
  size_type GetNumElements() {
    return num_elements_;
  }

  width_type GetBitWidth() {
    return bit_width_;
  }

  size_type size() const {
    return num_elements_;
  }

  size_type max_size() const {
    return num_elements_;
  }

  bool empty() const {
    return num_elements_ == 0;
  }

  // Serialization and De-serialization
  virtual size_type Serialize(std::ostream& out) override {
    size_t out_size = 0;

    out.write(reinterpret_cast<const char *>(&this->num_elements_),
              sizeof(size_type));
    out_size += sizeof(size_type);

    out.write(reinterpret_cast<const char *>(&this->bit_width_),
              sizeof(width_type));
    out_size += sizeof(width_type);

    out_size += Bitmap::Serialize(out);

    return out_size;
  }

  virtual size_type Deserialize(std::istream& in) override {
    size_t in_size = 0;

    in.read(reinterpret_cast<char *>(&this->num_elements_), sizeof(size_type));
    in_size += sizeof(size_type);

    in.read(reinterpret_cast<char *>(&this->bit_width_), sizeof(width_type));
    in_size += sizeof(width_type);

    in_size += Bitmap::Deserialize(in);

    return in_size;
  }

 protected:
  // Data members
  size_type num_elements_;
  width_type bit_width_;
};

// Unsigned bitmap array that does not store number of elements in order to
// save space; does not provide iterators as a consequence. Access/Modify with
// care, internal bound checks may not be possible
template<typename T>
class UnsizedBitmapArray : public Bitmap {
 public:
  // Type definitions
  typedef typename BitmapArray<T>::size_type size_type;
  typedef typename BitmapArray<T>::width_type width_type;
  typedef typename BitmapArray<T>::pos_type pos_type;

  typedef value_reference<UnsizedBitmapArray<T>> reference;
  typedef T value_type;

  // Constructors and destructors
  UnsizedBitmapArray()
      : Bitmap() {
    bit_width_ = 0;
  }

  UnsizedBitmapArray(const UnsizedBitmapArray& array) {
    data_ = array.data_;
    size_ = array.size_;
    bit_width_ = array.bit_width_;
  }

  UnsizedBitmapArray(size_type num_elements, width_type bit_width)
      : Bitmap(num_elements * bit_width) {
    bit_width_ = bit_width;
  }

  UnsizedBitmapArray(T *elements, size_type num_elements, width_type bit_width)
      : UnsizedBitmapArray(num_elements, bit_width) {

    for (uint64_t i = 0; i < num_elements; i++) {
      Set(i, elements[i]);
    }
  }

  // Accessors and mutators
  void Set(pos_type i, T value) {
    this->SetValPos(i * this->bit_width_, value, this->bit_width_);
  }

  T Get(pos_type i) const {
    return (T) this->GetValPos(i * this->bit_width_, this->bit_width_);
  }

  // Operators, iterators
  const T operator[](const pos_type& i) const {
    return Get(i);
  }

  reference operator[](const pos_type& i) {
    return reference(this, i);
  }

  // Serialization and De-serialization
  virtual size_type Serialize(std::ostream& out) override {
    size_t out_size = 0;

    out.write(reinterpret_cast<const char *>(&this->bit_width_),
              sizeof(width_type));
    out_size += sizeof(width_type);

    out_size += Bitmap::Serialize(out);

    return out_size;
  }

  virtual size_type Deserialize(std::istream& in) override {
    size_t in_size = 0;

    in.read(reinterpret_cast<char *>(&this->bit_width_), sizeof(width_type));
    in_size += sizeof(width_type);

    in_size += Bitmap::Deserialize(in);

    return in_size;
  }
 private:
  // Data members
  width_type bit_width_;
};

template<typename T>
class UnsignedBitmapArray : public BitmapArray<T> {
 public:
  static_assert(!std::numeric_limits<T>::is_signed,
      "Signed types cannot be used with UnsignedBitmapArray.");

  // Type definitions
  typedef typename BitmapArray<T>::size_type size_type;
  typedef typename BitmapArray<T>::width_type width_type;
  typedef typename BitmapArray<T>::pos_type pos_type;

  typedef ptrdiff_t difference_type;
  typedef T value_type;
  typedef T* pointer;
  typedef value_reference<UnsignedBitmapArray<T>> reference;
  typedef bitmap_array_iterator<UnsignedBitmapArray<T>> iterator;
  typedef const_bitmap_array_iterator<UnsignedBitmapArray<T>> const_iterator;
  typedef std::random_access_iterator_tag iterator_category;

  UnsignedBitmapArray()
      : BitmapArray<T>() {
  }

  UnsignedBitmapArray(size_type num_elements, width_type bit_width)
      : BitmapArray<T>(num_elements, bit_width) {
  }

  UnsignedBitmapArray(T *elements, size_type num_elements, width_type bit_width)
      : BitmapArray<T>(num_elements, bit_width) {

    for (uint64_t i = 0; i < this->num_elements_; i++) {
      Set(i, elements[i]);
    }
  }

  virtual ~UnsignedBitmapArray() {
  }

  // Accessors and mutators
  void Set(pos_type i, T value) {
    this->SetValPos(i * this->bit_width_, value, this->bit_width_);
  }

  T Get(pos_type i) const {
    return (T) this->GetValPos(i * this->bit_width_, this->bit_width_);
  }

  // Operators, iterators
  const T operator[](const pos_type& i) const {
    return Get(i);
  }

  reference operator[](const pos_type& i) {
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

  void swap(const UnsignedBitmapArray<T>& other) {
    using std::swap;
    swap(this->data_, other.data_);
    swap(this->size_, other.size_);
    swap(this->num_elements_, other.num_elements_);
    swap(this->bit_width_, other.bit_width_);
  }
};

template<typename T>
class SignedBitmapArray : public BitmapArray<T> {
 public:
  static_assert(std::numeric_limits<T>::is_signed,
      "Unsigned types should not be used with SignedBitmapArray.");

  // Type definitions
  typedef typename BitmapArray<T>::size_type size_type;
  typedef typename BitmapArray<T>::width_type width_type;
  typedef typename BitmapArray<T>::pos_type pos_type;

  typedef ptrdiff_t difference_type;
  typedef T value_type;
  typedef T* pointer;
  typedef value_reference<SignedBitmapArray<T>> reference;
  typedef bitmap_array_iterator<SignedBitmapArray<T>> iterator;
  typedef const_bitmap_array_iterator<SignedBitmapArray<T>> const_iterator;
  typedef std::random_access_iterator_tag iterator_category;

  SignedBitmapArray()
      : BitmapArray<T>() {
  }

  SignedBitmapArray(size_type num_elements, width_type bit_width)
      : BitmapArray<T>(num_elements, bit_width + 1) {
  }

  SignedBitmapArray(T *elements, size_type num_elements, width_type bit_width)
      : BitmapArray<T>(num_elements, bit_width + 1) {
    for (uint64_t i = 0; i < this->num_elements_; i++) {
      Set(i, elements[i]);
    }
  }

  virtual ~SignedBitmapArray() {
  }

  // Accessors and mutators
  void Set(pos_type i, T value) {
    if (value < 0) {
      this->SetValPos(i * this->bit_width_, ((-value) << 1) | 1,
                      this->bit_width_);
    } else {
      this->SetValPos(i * this->bit_width_, value << 1, this->bit_width_);
    }
  }

  T Get(pos_type i) const {
    T value = this->GetValPos(i * this->bit_width_, this->bit_width_);
    bool negate = (value & 1);
    return ((value >> 1) ^ -negate) + negate;
    // return (value & 1) ? -(value >> 1) : (value >> 1);
  }

  // Operators, iterators
  const T operator[](const pos_type& i) const {
    return Get(i);
  }

  reference operator[](const pos_type& i) {
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
    return iterator(this, this->num_elements_);
  }

  const_iterator cend() const {
    return iterator(this, this->num_elements_);
  }

  void swap(const SignedBitmapArray<T>& other) {
    using std::swap;
    swap(this->data_, other.data_);
    swap(this->size_, other.size_);
    swap(this->num_elements_, other.num_elements_);
    swap(this->bit_width_, other.bit_width_);
    swap(this->signs_, other.signs_);
  }
};

}
#endif