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

// Maximum number of bits for node configuration
typedef uint64_t node_type;
// Maximum number of nodes/preorder numbers
typedef uint64_t preorder_type;
typedef uint64_t n_leaves_type;
typedef uint16_t node_n_type;
typedef uint64_t level_type;
typedef uint64_t symbol_type;
typedef uint8_t dimension_type;
typedef uint64_t point_type;
extern uint64_t dfuds_size;
const preorder_type null_node = -1;

#define MAX_UINT_16 65535
#endif

// Struct for each point that we want to insert
class leaf_config
{
public:
    point_type *coordinates = nullptr;
    explicit leaf_config(dimension_type _dimensions)
    {
        coordinates = (point_type *)calloc(_dimensions, sizeof(point_type));
    }

    // Given the leaf_point and the level we are at, return the Morton code corresponding to that level
    symbol_type leaf_to_symbol(level_type _level, dimension_type _dimensions, level_type _max_depth)
    {
        symbol_type result = 0;
        for (dimension_type j = 0; j < _dimensions; j++)
        {
            bool bit = (coordinates[j] >> (_max_depth - _level - 1)) & 1;
            result = result << 1;
            result += bit;
        }
        return result;
    }
};

class leaf_array
{
public:
    n_leaves_type n_points = 0;
    leaf_config **points;
    leaf_array()
    {
        points = (leaf_config **)malloc(sizeof(leaf_config *));
    }
    void add_leaf(leaf_config *leaf){
        points[n_points] = leaf;
        n_points ++;
        points = (leaf_config **)realloc(points, (n_points + 1) * sizeof(leaf_config *));
    }
    void reset(){
        n_points = 0;
        free(points);
        points = (leaf_config **)malloc(sizeof(leaf_config *)); 
    }

};

// node_info and subtree info are used to obtain subtree size when splitting the treeblock
class node_info
{
public:
    preorder_type preorder_ = 0;
    preorder_type n_children_ = 0;
};
 
class subtree_info
{
public:
    preorder_type preorder_ = 0;
    preorder_type subtree_size_ = 0;
};

class trie_node
{
private:
    trie_node **children_ = nullptr;
public:
    void *block = nullptr;
    trie_node *get_child(symbol_type symbol){
        return children_[symbol];
    }

    void set_child(symbol_type symbol, trie_node *node){
        children_[symbol] = node;
    }

    explicit trie_node(symbol_type _n_branches)
    {
        // Assume null to be 0;
        children_ = (trie_node **)calloc(_n_branches, sizeof(trie_node *));
    }
    uint64_t size(symbol_type _n_branches) const;
};

class treeblock
{
private:
    uint8_t dimensions_;
    symbol_type n_branches_;
    node_n_type max_tree_nodes_;
    node_n_type initial_tree_capacity_;
    level_type root_depth_{};
    node_n_type n_nodes_{};
    node_n_type tree_capacity_{};
    level_type max_depth_;
    bitmap::Bitmap *dfuds_{};
    void *frontiers_ = nullptr;
    node_n_type n_frontiers_ = 0;
    
public: 
    explicit treeblock(uint8_t _dimensions, level_type _root_depth, node_n_type _tree_capacity, node_n_type _n_nodes, level_type _max_depth = 10, node_n_type _max_tree_nodes = 256, uint8_t initial_capacity_nodes = 8)
    {
        dimensions_ = _dimensions;
        root_depth_ = _root_depth;
        tree_capacity_ = _tree_capacity;
        n_branches_ = (symbol_type)pow(2, _dimensions);
        initial_tree_capacity_ = n_branches_ * initial_capacity_nodes;
        max_depth_ = _max_depth;
        max_tree_nodes_ = _max_tree_nodes;
        n_nodes_ = _n_nodes;
        dfuds_ = new bitmap::Bitmap((initial_tree_capacity_ + 1) * n_branches_);
    }
    
    node_n_type get_n_frontiers(){
        return n_frontiers_;
    }
    
    void insert(node_type, leaf_config *, level_type, level_type, preorder_type);
    node_type child(treeblock *&, node_type &, symbol_type, level_type &, preorder_type &) const;
    node_type skip_children_subtree(node_type &, symbol_type, level_type, preorder_type &) const;
    treeblock *get_pointer(preorder_type) const;
    preorder_type get_preorder(preorder_type) const;
    void set_preorder(preorder_type, preorder_type) const;
    void set_pointer(preorder_type, treeblock *) const;
    node_type select_subtree(preorder_type &, preorder_type &) const;
    uint64_t size() const;

    void range_search_treeblock(leaf_config *, leaf_config *, treeblock *, level_type, preorder_type, node_type, leaf_array *);
    void range_traverse_treeblock(leaf_config *, leaf_config *, uint8_t [], uint8_t, treeblock *, level_type, preorder_type, node_type, leaf_array *);

    preorder_type get_child_skip(node_type, symbol_type, symbol_type) const;
    preorder_type get_n_children(node_type, symbol_type) const;

    void copy_node_cod(bitmap::Bitmap *, bitmap::Bitmap *, node_type, node_type, symbol_type, symbol_type);
};

class frontier_node
{
public:
    preorder_type preorder_;
    treeblock *pointer_;
};

class md_trie
{
private:
    uint8_t dimensions_;
    symbol_type n_branches_;
    trie_node *root_ = nullptr;
    level_type max_depth_;
    level_type trie_depth_;
    preorder_type max_tree_nodes_;
    node_n_type initial_tree_capacity_;
public:
    explicit md_trie(uint8_t _dimensions, level_type _max_depth = 10, level_type _trie_depth = 3, preorder_type _max_tree_nodes = 256, uint8_t initial_capacity_nodes = 2)
    {
        dimensions_ = _dimensions;
        n_branches_ = (symbol_type)pow(2, _dimensions);
        initial_tree_capacity_ = n_branches_ * initial_capacity_nodes;
        max_depth_ = _max_depth;
        trie_depth_ = _trie_depth;
        max_tree_nodes_ = _max_tree_nodes;
        root_ = new trie_node(n_branches_);
    }
    trie_node *get_root(){
        return root_;
    }
    bool check(leaf_config *, level_type) const;
    void insert_remaining(treeblock *, leaf_config *, level_type, level_type) const;
    void insert_trie(leaf_config *, level_type);
    treeblock *walk_trie(trie_node *, leaf_config *, level_type &) const;
    bool walk_treeblock(treeblock *, leaf_config *, level_type, level_type) const;
    uint64_t size() const;
    void range_search_trie(leaf_config *, leaf_config *, trie_node *, level_type, leaf_array *);
    void range_traverse_trie(leaf_config *, leaf_config *, uint8_t [], uint8_t, trie_node *, level_type, leaf_array *);
};



