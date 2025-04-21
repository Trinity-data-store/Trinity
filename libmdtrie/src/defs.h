#ifndef MD_TRIE_DEFS_H
#define MD_TRIE_DEFS_H

#include "compact_vector.h"
#include <assert.h>
#include <boost/bimap.hpp>
#include <cinttypes>
#include <compressed_bitmap.h>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <unordered_map>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint64_t preorder_t;
typedef uint64_t n_leaves_t;
typedef uint64_t node_pos_t;
typedef uint8_t level_t;
typedef uint64_t morton_t;
typedef uint64_t dimension_t;
typedef uint64_t point_t;

const preorder_t null_node = -1;

template <dimension_t DIMENSION>
class tree_block;

template <dimension_t DIMENSION>
class data_point;

/**
 * node_info and subtree info are used when splitting the treeblock to create a
 * new frontier node node_info stores the additional information of number of
 * children each node has subtree_info stores the additional information of the
 * subtree size of that node
 */

struct node_info
{
  preorder_t preorder_ = 0;
  preorder_t n_children_ = 0;
};

struct subtree_info
{
  preorder_t preorder_ = 0;
  preorder_t subtree_size_ = 0;
};

template <dimension_t DIMENSION>
struct frontier_node
{
  preorder_t preorder_;
  tree_block<DIMENSION> *pointer_;
};

typedef unsigned long long int TimeStamp;

TimeStamp
GetTimestamp()
{
  struct timeval now;
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp)now.tv_sec * 1000000;
}

/**
 * total_points_count: total number of points in the data set
 * discount_factor: only consider total_points_count/discount_factor number of
 * points total_treeblock_num: total number of treeblocks in MdTrie. Used for
 * size calculation level_to_num_children: maps level to number of children a
 * node has at that level
 */

n_leaves_t total_points_count = 0;
int discount_factor = 1;
level_t trie_depth_;
dimension_t trie_width_;
morton_t level_to_num_children[80] = {0};
// for faster indexing with flexible widths
std::vector<std::vector<std::pair<dimension_t, level_t>>> _dim_off_table;
preorder_t max_tree_nodes_ = 512;
level_t max_depth_;

/**
 * p_key_to_treeblock_compact: maps primary key to treeblock pointers in a
 * compact pointer vector dimension_to_num_bits: maps the attribute index to bit
 * widths of that attribute start_dimension_bits: the level to which we start
 * considering bits from that attribute no_dynamic_sizing: flag to indicate
 * whether we set the treeblock size to the same value.
 */

bitmap::CompactPtrVector *p_key_to_treeblock_compact;
std::vector<level_t> dimension_to_num_bits;
std::vector<level_t> start_dimension_bits;
bool no_dynamic_sizing = false;
std::map<void *, void *> old_ptr_to_new_ptr;
std::map<void *, size_t> ptr_to_file_offset;
size_t current_file_offset = 0;
n_leaves_t treeblock_ctr = 0;

std::mutex cache_lock;

std::unordered_map<int32_t, int32_t> client_to_server;
bool enable_client_cache_pkey_mapping = false;
bool REUSE_RANGE_SEARCH_CHILD = true;

int current_dataset_idx = 0;
int num_treeblock_expand = 0;
int lookup_scanned_nodes = 0;

uint64_t bare_minimum_count = 0;
uint64_t checked_points_count = 0;
int query_optimization = 2;
bool is_collapsed_node_exp = false;

// build a ragged 2D “dim_off” table: _dim_off_table[level][i] = {dim,offset}
void create_dim_off_table(const std::vector<level_t>& bit_widths,
                          const std::vector<level_t>& start_bits,
                          dimension_t W) {

  _dim_off_table.clear();

  dimension_t D = (dimension_t) bit_widths.size();
  assert(D == dimension_t(start_bits.size()));
  assert(W > 0);

  // compute how many bits each dim really contributes, and total
  std::vector<level_t> rem(D);
  level_t total = 0, max_rem = 0;
  for (uint16_t d = 0; d < D; ++d) {
    assert(bit_widths[d] >= start_bits[d]);
    rem[d] = bit_widths[d] - start_bits[d];
    total += rem[d];
    max_rem = std::max(max_rem, rem[d]);
  }

  // prepare the ragged 2D result
  _dim_off_table.emplace_back();
  uint16_t in_this_level = 0;

  // emit in “global bit‐rounds” order, grouping every W bits into one level
  for (level_t g = 0; g < max_rem; ++g) {
    for (dimension_t d = 0; d < D; ++d) {
      if (g >= start_bits[d] && g < start_bits[d] + rem[d]) {
        // compute offset of bit g in dimension d
        level_t offset = bit_widths[d] - 1 - g;

        // if current level is full, start a new one
        if (in_this_level == W) {
          _dim_off_table.emplace_back();
          in_this_level = 0;
        }

        _dim_off_table.back().emplace_back(d, offset);
        ++in_this_level;
      }
    }
  }

  // sanity check: we should have emitted exactly 'total' bits
  int check = 0;
  for (auto &lvl : _dim_off_table) check += int(lvl.size());
  assert(check == total);
}

void create_level_to_num_children(std::vector<level_t> bit_widths,
                                  std::vector<level_t> start_bits,
                                  level_t max_level,
                                  dimension_t width)
{
  for (int i = 0; i < 80; i++) {
    level_to_num_children[i] = 0;
  }

  dimension_to_num_bits = bit_widths;
  start_dimension_bits = start_bits;

  create_dim_off_table(dimension_to_num_bits, start_dimension_bits, width);

  max_depth_ = (level_t) _dim_off_table.size();

  assert(max_depth_ == max_level);

  for (level_t lvl = 0; lvl < max_depth_; lvl++) {
    level_to_num_children[lvl] = _dim_off_table[lvl].size();
  }

  // sanity check
  dimension_t num_dimensions = bit_widths.size();

  uint16_t total_bits = 0;

  for (dimension_t i = 0; i < num_dimensions; i++) {
    total_bits += (dimension_to_num_bits[i] - start_dimension_bits[i]);
  }

  for (level_t lvl = 0; lvl < max_depth_; lvl++) {
    total_bits -= level_to_num_children[lvl];
  }

  assert(total_bits == 0);
}

// #define USE_LINEAR_SCAN // Possibly disable at microbenchmark
#endif // MD_TRIE_DEFS_H
