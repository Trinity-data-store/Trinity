#ifndef MD_TRIE_MD_TRIE_H
#define MD_TRIE_MD_TRIE_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <utility>
#include <cstdint>
#include <ctime>
#include <csignal>
#include "bitmap.h"
#include <sys/stat.h>
#include <vector>
#include "defs.h"
#include "data_point.h"
#include "point_array.h"
#include "tree_block.h"
#include "trie_node.h"

class md_trie {
public:
    explicit md_trie(uint8_t dimensions, level_t max_depth = 10, level_t trie_depth = 3,
                     preorder_t max_tree_nodes = 256, uint8_t initial_capacity_nodes = 2) {
        dimensions_ = dimensions;
        n_branches_ = (symbol_t) pow(2, dimensions);
        initial_tree_capacity_ = n_branches_ * initial_capacity_nodes;
        max_depth_ = max_depth;
        trie_depth_ = trie_depth;
        max_tree_nodes_ = max_tree_nodes;
        root_ = new trie_node(n_branches_);
    }

    trie_node *root() {
        return root_;
    }

    bool check(data_point *, level_t) const;

    void insert_remaining(tree_block *, data_point *, level_t, level_t) const;

    void insert_trie(data_point *, level_t);

    tree_block *walk_trie(trie_node *, data_point *, level_t &) const;

    bool walk_tree_block(tree_block *current_block, data_point *leaf_point, level_t length, level_t level) const;

    uint64_t size() const;

    void range_search_trie(data_point *, data_point *, trie_node *, level_t, point_array *, uint8_t *);

    void range_traverse_trie(data_point *, data_point *, uint8_t [], uint8_t, trie_node *, level_t, point_array *);

private:
    uint8_t dimensions_;
    symbol_t n_branches_;
    trie_node *root_ = nullptr;
    level_t max_depth_;
    level_t trie_depth_;
    preorder_t max_tree_nodes_; // TODO(anuragk): Not used
    node_n_t initial_tree_capacity_;
};


#endif //MD_TRIE_MD_TRIE_H
