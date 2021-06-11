#pragma once
#ifndef TREEBLOCK
#define TREEBLOCK

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <utility>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include "bitmap.h"
#include <sys/stat.h>

using namespace std;

// Naming styles:
// class: md_tire
// function: walk_trie
// local variable names: n_branches
// class variable: n_branches_

// Maximum number of bits for node configuration
typedef uint64_t node_type;
// Maximum number of nodes/preorder numbers
typedef uint64_t preorder_type;
typedef uint16_t node_n_type;
typedef uint64_t level_type;
typedef uint64_t symbol_type;
typedef uint8_t dimension_type;
typedef uint64_t point_type;
extern uint64_t dfuds_size;
// Depth of the trie (leaves are pointers to treeblocks)
// const level_type trie_depth = 3;
// Maximum number of nodes in a treeblock
// const preorder_type initial_tree_capacity = 32;
// const preorder_type max_tree_nodes = 256;
const preorder_type null_node = -1;
// MAX_DPETH specifies the range of each dimension (0 to 2^MAX_DEPTH -1)
// const level_type max_depth = 10;
#define MAX_UINT_16 65535
#endif

// Struct for each point that we want to insert
class leaf_config
{
public:
    point_type *coordinates = NULL;
    leaf_config(dimension_type _dimensions)
    {
        coordinates = (point_type *)calloc(_dimensions, sizeof(point_type));
    }
};

// class search_range
// {
// public:
//     point_type *from = NULL;
//     point_type *to = NULL;
//     leaf_config(dimension_type _dimensions)
//     {
//         from = (point_type *)calloc(_dimensions, sizeof(point_type));
//         to = (point_type *)calloc(_dimensions, sizeof(point_type));
//     }
// };


// node_info and subtree info are used to obtain subtree size when splitting the treeblock
class node_info
{
public:
    preorder_type preorder_ = 0;
    preorder_type n_children_ = 0;
    node_info(){};
    node_info(preorder_type _preorder, preorder_type _n_children)
    {
        preorder_ = _preorder;
        n_children_ = _n_children;
    };
};
 
class subtree_info
{
public:
    preorder_type preorder_ = 0;
    preorder_type subtree_size_ = 0;
    subtree_info(){};
    subtree_info(preorder_type _preorder, preorder_type _subtree_size)
    {
        preorder_ = _preorder;
        subtree_size_ = _subtree_size;
    };
};

class trie_node
{
public:
    trie_node **children_ = NULL;
    trie_node(symbol_type _n_branches)
    {
        // Assume null to be 0;
        children_ = (trie_node **)calloc(_n_branches, sizeof(trie_node *));
    }
    void *block = NULL;
    uint64_t size(symbol_type _n_branches);
};

class treeblock
{
public: 
    uint8_t dimensions_;
    symbol_type n_branches_;

    level_type root_depth_;
    node_n_type n_nodes_;
    node_n_type tree_capacity_;
    level_type max_depth_;
    bitmap::Bitmap *dfuds_;
    void *frontiers_ = NULL;
    node_n_type n_frontiers_ = 0;
    node_n_type max_tree_nodes_;

    node_n_type initial_tree_capacity_;
    treeblock(uint8_t _dimensions, level_type _max_depth = 10, node_n_type _max_tree_nodes = 256, uint8_t initial_capacity_nodes = 8)
    {
        dimensions_ = _dimensions;
        n_branches_ = pow(2, _dimensions);
        initial_tree_capacity_ = n_branches_ * initial_capacity_nodes;
        max_depth_ = _max_depth;
        max_tree_nodes_ = _max_tree_nodes;
    }

    void insert(node_type, leaf_config *, level_type, level_type, preorder_type);
    node_type child(treeblock *&, node_type &, symbol_type, level_type &, preorder_type &);
    node_type skip_children_subtree(node_type &, symbol_type, level_type, preorder_type &);
    struct treeblock *get_pointer(preorder_type);
    preorder_type get_preorder(preorder_type);
    void set_preorder(preorder_type, preorder_type);
    void set_pointer(preorder_type, treeblock *);
    node_type select_subtree(preorder_type &, preorder_type &);
    uint64_t size();

    void range_search_treeblock(leaf_config *, leaf_config *, treeblock *, level_type, preorder_type, node_type);
    void range_traverse_treeblock(leaf_config *, leaf_config *, int [], int, treeblock *, level_type, preorder_type, node_type);
};

class frontier_node
{
public:
    preorder_type preorder_;
    treeblock *pointer_;
};


class md_trie
{
public:
    uint8_t dimensions_;
    symbol_type n_branches_;
    trie_node *root_ = NULL;
    level_type max_depth_;
    level_type trie_depth_;
    preorder_type max_tree_nodes_;
    node_n_type initial_tree_capacity_;

    md_trie(uint8_t _dimensions, level_type _max_depth = 10, level_type _trie_depth = 3, preorder_type _max_tree_nodes = 256, uint8_t initial_capacity_nodes = 2)
    {
        dimensions_ = _dimensions;
        n_branches_ = pow(2, _dimensions);
        initial_tree_capacity_ = n_branches_ * initial_capacity_nodes;
        max_depth_ = _max_depth;
        trie_depth_ = _trie_depth;
        max_tree_nodes_ = _max_tree_nodes;
    }
    bool check(leaf_config *, level_type);
    void insert_remaining(treeblock *, leaf_config *, level_type, level_type);
    void insert_trie(leaf_config *, level_type);
    treeblock *walk_trie(trie_node *, leaf_config *, level_type &);
    bool walk_treeblock(treeblock *, leaf_config *, level_type, level_type);
    trie_node *create_new_trie_node();
    uint64_t size();
    void range_search_trie(leaf_config *, leaf_config *, trie_node *, level_type);
    void range_traverse_trie(leaf_config *, leaf_config *, int [], int, trie_node *, level_type);
};

// Todo: better arrangement of these functions
// See how create_new_trie_node is in a different place than create_new_treeblock
treeblock *create_new_treeblock(level_type, preorder_type, preorder_type, int);
symbol_type leaf_to_symbol(leaf_config *, level_type, int, level_type);
preorder_type get_child_skip(bitmap::Bitmap *, node_type, symbol_type, symbol_type);
preorder_type get_n_children(bitmap::Bitmap *, node_type, symbol_type);
void copy_node_cod(bitmap::Bitmap *, bitmap::Bitmap *, node_type, node_type, symbol_type, symbol_type);
