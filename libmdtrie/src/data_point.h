#ifndef MD_TRIE_DATA_POINT_H
#define MD_TRIE_DATA_POINT_H

#include "compressed_bitmap.h"
#include "defs.h"
#include <cstdlib>
#include <cstring>

template <dimension_t DIMENSION>
class data_point
{
public:
  explicit data_point() {}

  inline point_t *get() { return coordinates_; }

  inline void set(point_t *coordinates)
  {

    memcpy(coordinates_, coordinates, sizeof(point_t) * DIMENSION);
  }

  inline point_t get_coordinate(dimension_t index)
  {

    return coordinates_[index];
  }

  inline void set_coordinate(dimension_t index, point_t value)
  {

    coordinates_[index] = value;
  }

  inline void set_primary(n_leaves_t value) { primary_key_ = value; }

  inline n_leaves_t read_primary() { return primary_key_; }

  inline morton_t leaf_to_symbol(level_t level)
  {

    morton_t result = 0;
    auto &tbl = _dim_off_table[level];

    for (uint16_t i = 0; i < tbl.size(); i++) {

      auto [dim, off] = tbl[i];

      point_t coordinate = coordinates_[dim];

      bool bit = GETBIT (coordinate, off);
      result = (result << 1U) + bit;
    }
    
    return result;
  }

  inline morton_t leaf_to_full_symbol(level_t level)
  {

    morton_t result = 0;
    auto &tbl = _dim_off_table[level];

    for (uint16_t i = 0; i < tbl.size(); i++) {
      bool bit = 1;
      result = (result << 1U) + bit;
    }

    return result;
  }

  // NOT USED ANYWHERE, but
  // might need to change 32 to max_depth of trie
  uint64_t coordinate_to_raw_morton()
  {

    uint64_t result = 0;

    for (level_t level = 0; level < 32; level++)
    {
      for (size_t i = 0; i < DIMENSION; i++)
      {
        level_t offset = DIMENSION - level - 1;
        point_t coordinate = coordinates_[i];

        bool bit = GETBIT(coordinate, offset);
        result = (result << 1U) + bit;
      }
    }
    return result;
  }

  inline void update_symbol(data_point *end_range,
                            morton_t current_symbol,
                            level_t level)
  {

    // dimension_t dimension = DIMENSION;
    size_t visited_ct = 0;
    auto &tbl = _dim_off_table[level];
    dimension_t bits_in_lvl = (dimension_t) tbl.size();

    for (uint16_t i = 0; i < bits_in_lvl; i++) {

      auto [dim, off] = tbl[i];
      visited_ct++;

      dimension_t sym_off = bits_in_lvl - visited_ct;

      bool start_bit  = GETBIT(coordinates_[dim], off);
      bool end_bit    = GETBIT(end_range->coordinates_[dim], off);
      bool sym_bit    = GETBIT(current_symbol, sym_off);

      // adjust the range endpoints:
      if (sym_bit && !start_bit) {
          auto v = coordinates_[dim] & low_bits_unset[off];
          SETBIT(v, off);
          coordinates_[dim] = v;
      }
      if (!sym_bit && end_bit) {
          auto v = end_range->coordinates_[dim] | low_bits_set[off];
          CLRBIT(v, off);
          end_range->coordinates_[dim] = v;
      }
    }
  }

  std::vector<int32_t> generate_vector()
  {
    std::vector<int32_t> return_pt(DIMENSION);

    for (unsigned i = 0; i < DIMENSION; i++)
    {
      return_pt[i] = coordinates_[i];
    }

    return return_pt;
  }

private:
  /**
   * Data point struct is used both to represent a data point and the return
   * "struct" for range search Range search can either return a vector of
   * coordinates or a vector of primary keys
   */
  // dimension_t width_;
  point_t coordinates_[DIMENSION] = {0};
  n_leaves_t primary_key_ = 0;
};

#endif // MD_TRIE_DATA_POINT_H
