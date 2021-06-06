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
typedef uint64_t level_type;
typedef uint64_t symbol_type;
typedef uint8_t dimension_type;
typedef uint64_t point_type;

// Depth of the trie (leaves are pointers to treeblocks)
const level_type trie_depth = 8;
// Maximum number of nodes in a treeblock
const preorder_type max_tree_nodes = 1024;
const preorder_type null_node = -1;
// MAX_DPETH specifies the range of each dimension (0 to 2^MAX_DEPTH -1)
// const level_type max_depth = 10;

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
        children_ = (trie_node **)malloc(_n_branches * sizeof(trie_node *));
    }
    void *block;
};

class treeblock
{
public:
    int dimensions_;
    symbol_type n_branches_;

    level_type root_depth_;
    preorder_type n_nodes_;
    preorder_type max_nodes_;
    level_type max_depth_;
    bitmap::Bitmap *dfuds_;
    void *frontiers_;
    preorder_type n_frontiers_;

    treeblock(int _dimensions, level_type _max_depth = 10)
    {
        dimensions_ = _dimensions;
        n_branches_ = pow(2, _dimensions);
        max_depth_ = _max_depth;
    }

    void insert(node_type, leaf_config *, level_type, level_type, preorder_type);
    node_type child(treeblock *&, node_type &, symbol_type, level_type &, preorder_type &);
    node_type skip_children_subtree(node_type &, symbol_type, level_type, preorder_type &);
    struct treeblock *get_pointer(preorder_type);
    preorder_type get_preorder(preorder_type);
    void set_preorder(preorder_type, preorder_type);
    void set_pointer(preorder_type, treeblock *);
    node_type select_subtree(preorder_type &, preorder_type &);
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
    int dimensions_;
    symbol_type n_branches_;
    trie_node *root_ = NULL;
    level_type max_depth_;
    md_trie(int _dimensions, level_type _max_depth = 10)
    {
        dimensions_ = _dimensions;
        n_branches_ = pow(2, _dimensions);
        max_depth_ = _max_depth;
    }
    bool check(leaf_config *, level_type);
    void insert_remaining(treeblock *, leaf_config *, level_type, level_type);
    void insert_trie(leaf_config *, level_type);
    treeblock *walk_trie(trie_node *, leaf_config *, level_type &);
    bool walk_treeblock(treeblock *, leaf_config *, level_type, level_type);
    trie_node *create_new_trie_node();
};

// Todo: better arrangement of these functions
// See how create_new_trie_node is in a different place than create_new_treeblock
treeblock *create_new_treeblock(level_type, preorder_type, preorder_type, int);
symbol_type leaf_to_symbol(leaf_config *, level_type, int, level_type);
preorder_type get_child_skip(bitmap::Bitmap *, node_type, symbol_type, symbol_type);
preorder_type get_n_children(bitmap::Bitmap *, node_type, symbol_type);
void copy_node_cod(bitmap::Bitmap *, bitmap::Bitmap *, node_type, node_type, symbol_type);
