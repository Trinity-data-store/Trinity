#ifndef MD_TRIE_DEFS_H
#define MD_TRIE_DEFS_H

#include <cinttypes>
#include <vector>

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
// typedef std::vector<point_t> coordinates_t;
typedef point_t * coordinates_t;
typedef std::vector<uint64_t> density_array;

extern uint64_t dfuds_size;
const preorder_t null_node = -1;
extern uint64_t get_bit_count;
// typedef unsigned long long int TimeStamp;
// extern TimeStamp backtrace_time;

template<dimension_t DIMENSION>
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

template<dimension_t DIMENSION>
struct frontier_node {
    preorder_t preorder_;
    tree_block<DIMENSION> *pointer_;
};

#endif //MD_TRIE_DEFS_H
