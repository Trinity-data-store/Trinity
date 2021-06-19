#ifndef MD_TRIE_TREE_BLOCK_H
#define MD_TRIE_TREE_BLOCK_H

#include "point_array.h"
#include "bitmap.h"
#include <cmath>

class tree_block {
public:
    explicit tree_block(uint8_t dimensions, level_t root_depth, node_n_t tree_capacity, node_n_t num_nodes,
                        level_t max_depth = 10, node_n_t max_tree_nodes = 256, uint8_t initial_capacity_nodes = 8) {
        dimensions_ = dimensions;
        root_depth_ = root_depth;
        tree_capacity_ = tree_capacity;
        num_branches_ = (symbol_t) pow(2, dimensions);
        initial_tree_capacity_ = num_branches_ * initial_capacity_nodes;
        max_depth_ = max_depth;
        max_tree_nodes_ = max_tree_nodes;
        num_nodes_ = num_nodes;
        dfuds_ = new bitmap::Bitmap((initial_tree_capacity_ + 1) * num_branches_);
    }

    node_n_t num_frontiers() {
        return num_frontiers_;
    }

    void insert(node_t, data_point *, level_t, level_t, preorder_t);

    node_t child(tree_block *&, node_t &, symbol_t, level_t &, preorder_t &) const;

    node_t skip_children_subtree(node_t &, symbol_t, level_t, preorder_t &) const;

    tree_block *get_pointer(preorder_t) const;

    preorder_t get_preorder(preorder_t) const;

    void set_preorder(preorder_t, preorder_t);

    void set_pointer(preorder_t, tree_block *);

    node_t select_subtree(preorder_t &, preorder_t &) const;

    uint64_t size() const;

    void density(density_array *);

    void range_search_treeblock(data_point *, data_point *, tree_block *, level_t, preorder_t, node_t,
                                point_array *);

    void
    range_traverse_treeblock(data_point *, data_point *, uint8_t [], uint8_t, tree_block *, level_t, preorder_t,
                             node_t, point_array *);

    preorder_t get_child_skip(node_t, symbol_t, symbol_t) const;

    preorder_t get_n_children(node_t, symbol_t) const;

    static void copy_node_cod(bitmap::Bitmap *, bitmap::Bitmap *, node_t, node_t, symbol_t, symbol_t);

private:
    uint8_t dimensions_;
    symbol_t num_branches_;
    node_n_t max_tree_nodes_;
    node_n_t initial_tree_capacity_;
    level_t root_depth_{};
    node_n_t num_nodes_{};
    node_n_t tree_capacity_{};
    level_t max_depth_;
    bitmap::Bitmap *dfuds_{};
    // TODO(anuragk): Should be replaced by std::vector<frontier_node> if possible
    frontier_node *frontiers_ = nullptr; 
    node_n_t num_frontiers_ = 0;
};

#endif //MD_TRIE_TREE_BLOCK_H
