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

typedef uint64_t n_leaves_t;
typedef uint8_t level_t;
typedef uint64_t morton_t;
typedef uint64_t dimension_t;

const n_leaves_t null_node = -1;

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
  n_leaves_t preorder_ = 0;
  n_leaves_t n_children_ = 0;
};

struct subtree_info
{
  n_leaves_t preorder_ = 0;
  n_leaves_t subtree_size_ = 0;
};

template <dimension_t DIMENSION>
struct frontier_node
{
  n_leaves_t preorder_;
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
 * points total_treeblock_num: total number of treeblocks in MdTrie. Used for
 * size calculation
 */

n_leaves_t total_points_count = 0;
level_t trie_depth_;
level_t max_depth_;

// For Trinity
std::unordered_map<int32_t, int32_t> client_to_server;

/* For toggling Collapsed Node optimization */
bool is_collapsed_node_exp = false;

// #define USE_LINEAR_SCAN // Possibly disable at microbenchmark
#endif // MD_TRIE_DEFS_H
