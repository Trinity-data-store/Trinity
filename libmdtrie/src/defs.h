#ifndef MD_TRIE_DEFS_H
#define MD_TRIE_DEFS_H

#include <cinttypes>
#include <vector>
#include <bimap.h>
#include <compressed_bitmap.h>
#include <assert.h> 
#include <boost/bimap.hpp>
#include <mutex>
#include <shared_mutex>
#include "compact_vector.h"

// Maximum number of bits for node configuration
typedef uint64_t node_t;
// Maximum number of nodes/preorder numbers
typedef uint64_t preorder_t;
typedef uint64_t n_leaves_t;
typedef uint16_t node_n_t;
typedef uint64_t level_t;
typedef uint64_t symbol_t;
typedef uint8_t dimension_t;
typedef uint64_t point_t;
typedef uint16_t representation_t;
typedef point_t * coordinates_t;
typedef std::vector<uint64_t> density_array;

extern uint64_t dfuds_size;
const preorder_t null_node = -1;

extern uint64_t get_bit_count;
extern uint64_t v2_storage_save_pos;
extern uint64_t v2_storage_save_neg;
extern uint64_t single_node_count;
extern uint64_t total_number_nodes; 

class tree_block;

// node_info and subtree info are used to obtain subtree size when splitting the treeblock
struct node_info {
    preorder_t preorder_ = 0;
    preorder_t n_children_ = 0;
};

struct subtree_info {
    preorder_t preorder_ = 0;
    preorder_t subtree_size_ = 0;
};

struct frontier_node {
    preorder_t preorder_;
    tree_block *pointer_;
};

typedef unsigned long long int TimeStamp;

static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}


n_leaves_t current_leaves_inserted = 0;
uint64_t max_count = 0;

TimeStamp vector_time = 0;
uint64_t vect_opt_count = 0;
std::shared_mutex mutex_p_key;

n_leaves_t total_points_count = 14252681;


bitmap::CompactPtrVector p_key_to_treeblock_compact(total_points_count);

uint64_t trie_size = 0;
uint64_t vector_size = 0;
uint64_t treeblock_ptr_size = 0;
uint64_t treeblock_nodes_size = 0;
uint64_t total_leaf_number = 0;

TimeStamp primary_time = 0;
class data_point;

std::map<uint64_t, uint64_t> node_children_to_occurrences;
bool is_osm = false;

uint64_t thrift_vector_time = 0;
uint64_t thrift_inner_function_time = 0;
uint64_t top_trie_range_search_latency = 0;
uint64_t treeblock_range_search_latency = 0;
uint64_t update_range_latency = 0;
uint64_t child_latency = 0;

std::vector<symbol_t> level_to_num_children;
std::vector<symbol_t> dimension_to_num_bits;

void create_level_to_num_children(std::vector<level_t> dimension_bits, level_t max_level){

    dimension_t num_dimensions = dimension_bits.size();

    for (level_t level = 0; level < max_level; level++){

        dimension_t dimension_left = num_dimensions;
        for (dimension_t j = 0; j < num_dimensions; j++){
            
            if (level + 1 > dimension_bits[j])
                dimension_left --;
        }
        level_to_num_children.push_back(dimension_left);
    }
}

unsigned int reverse(unsigned int b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

unsigned int bit_width(uint64_t val){
    return 64 - __builtin_clzll(val);
}

bool toggle = false;
#endif //MD_TRIE_DEFS_H
