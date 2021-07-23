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
#include <mutex>
// std::mutex mutex;

template<dimension_t DIMENSION>
class md_trie {
public:
    explicit md_trie(level_t max_depth, level_t trie_depth,
                     preorder_t max_tree_nodes, uint8_t initial_capacity_nodes = 8) {
        n_branches_ = (symbol_t) pow(2, DIMENSION);
        initial_tree_capacity_ = initial_capacity_nodes;
        max_depth_ = max_depth;
        trie_depth_ = trie_depth;
        max_tree_nodes_ = max_tree_nodes;
        root_ = new trie_node<DIMENSION>(n_branches_);
    }

    inline trie_node<DIMENSION> *root() {
        return root_;
    }

    // Traverse the current TreeBlock, going into frontier nodes as needed
    // Until it cannot traverse further and calls insertion
    void insert_remaining(tree_block<DIMENSION> *root, data_point<DIMENSION> *leaf_point, level_t length, level_t level) const {

        // mutex.lock();
        tree_block<DIMENSION> *current_block = root;
        node_t current_node = 0;
        preorder_t current_frontier = 0;
        
        node_t temp_node = 0;
        while (level < length) {
            temp_node = current_block->child(current_block, current_node,
                                            leaf_point->leaf_to_symbol(level, max_depth_), level,
                                            current_frontier);
            if (temp_node == (node_t) -1)
                break;
            current_node = temp_node;
            if (current_block->num_frontiers() > 0 && current_frontier < current_block->num_frontiers() &&
                current_node == current_block->get_preorder(current_frontier)) {
                current_block = current_block->get_pointer(current_frontier);
                current_node = (node_t) 0;
                current_frontier = 0;
            }
            level++;
        }
        
        current_block->insert(current_node, leaf_point, level, length, current_frontier);
        // mutex.unlock();
    }

    // This function is used for testing.
    // It differs from above as it only returns True or False.
    bool walk_tree_block(tree_block<DIMENSION> *current_block, data_point<DIMENSION> *leaf_point, level_t length, level_t level) const {
        preorder_t current_frontier = 0;
        node_t current_node = 0;
        node_t temp_node = 0;
        while (level < length) {
            temp_node = current_block->child(current_block, current_node,
                                            leaf_point->leaf_to_symbol(level, max_depth_), level,
                                            current_frontier);
            if (temp_node == (node_t) -1)
                return false;
            current_node = temp_node;

            if (current_block->num_frontiers() > 0 && current_frontier < current_block->num_frontiers() &&
                current_node == current_block->get_preorder(current_frontier)) {
                current_block = current_block->get_pointer(current_frontier);
                current_node = (node_t) 0;
                current_frontier = 0;
            }
            level++;
        }
        return true;
    }

    // This function goes down the trie
    // Return the treeblock at the leaf of the trie
    tree_block<DIMENSION> *walk_trie(trie_node<DIMENSION> *current_trie_node, data_point<DIMENSION> *leaf_point, level_t &level) const {
        symbol_t current_symbol;
        while (current_trie_node->get_child(leaf_point->leaf_to_symbol(level, max_depth_)))
            current_trie_node = current_trie_node->get_child(leaf_point->leaf_to_symbol(level++, max_depth_));
        while (level < trie_depth_) {
            current_symbol = leaf_point->leaf_to_symbol(level, max_depth_);
            current_trie_node->set_child(current_symbol, new trie_node<DIMENSION>(n_branches_));
            current_trie_node->get_child(current_symbol)->parent_trie_node = current_trie_node;
            current_trie_node->get_child(current_symbol)->parent_symbol = current_symbol;
            current_trie_node = current_trie_node->get_child(current_symbol);
            level++;
        }
        tree_block<DIMENSION> *current_treeblock = nullptr;
        if (current_trie_node->block() == nullptr) {
            current_treeblock = new tree_block<DIMENSION>(trie_depth_, initial_tree_capacity_, 1, max_depth_, max_tree_nodes_);
            current_trie_node->block(current_treeblock);
            current_treeblock->set_parent_trie_node(current_trie_node);
        } else
            current_treeblock = (tree_block<DIMENSION> *) current_trie_node->block();
        return current_treeblock;
    }

    // This function inserts a string into a trie_node.
    // The first part it traverses is the trie, followed by traversing the treeblock
    void insert_trie(data_point<DIMENSION> *leaf_point, level_t length) {

        
        if (root_ == nullptr) {
            root_ = new trie_node<DIMENSION>(n_branches_);
        }
        level_t level = 0;
        trie_node<DIMENSION> *current_trie_node = root_;
        tree_block<DIMENSION> *current_treeblock = walk_trie(current_trie_node, leaf_point, level);

        
        insert_remaining(current_treeblock, leaf_point, length, level);
        
    }

    // Used for Test script to check whether a leaf_point is present
    bool check(data_point<DIMENSION> *leaf_point, level_t strlen) const {
        level_t level = 0;
        trie_node<DIMENSION> *current_trie_node = root_;
        tree_block<DIMENSION> *current_treeblock = walk_trie(current_trie_node, leaf_point, level);
        return walk_tree_block(current_treeblock, leaf_point, strlen, level);
    }

    uint64_t size() const {
        uint64_t total_size = sizeof(uint8_t) + sizeof(symbol_t) + sizeof(trie_node<DIMENSION> *) + sizeof(level_t) * 2 +
                            sizeof(preorder_t) * 2;
        return total_size + root_->size();
    }

    void density(density_array *array) {
        root_->density(array);
    }

    void range_search_trie(data_point<DIMENSION> *start_range, data_point<DIMENSION> *end_range, trie_node<DIMENSION> *current_trie_node,
                                    level_t level, point_array<DIMENSION> *found_points) {
        // If we reach the bottom of the top-level trie
        if (level == trie_depth_) {
            auto *current_treeblock = (tree_block<DIMENSION> *) current_trie_node->block();
            current_treeblock->range_search_treeblock(start_range, end_range, current_treeblock, level, 0, 0, 0, found_points);
            return;
        }
        symbol_t start_morton = start_range->leaf_to_symbol(level, max_depth_);
        symbol_t end_morton = end_range->leaf_to_symbol(level, max_depth_);
        symbol_t representation = start_morton ^ end_morton;
        symbol_t neg_representation = ~representation;

        struct data_point<DIMENSION> original_start_range = (*start_range);
        struct data_point<DIMENSION> original_end_range = (*end_range); 
        
        for (symbol_t current_morton = 0; current_morton < n_branches_; current_morton++){

            if ((start_morton & neg_representation) != (current_morton & neg_representation)){
                continue;
            }
            
            if (!current_trie_node->get_child(current_morton)) {
                continue;
            }

            start_range->update_range_morton(end_range, current_morton, level, max_depth_);

            range_search_trie(start_range, end_range, current_trie_node->get_child(current_morton), level + 1,
                                found_points);

            (*start_range) = original_start_range;
            (*end_range) = original_end_range;                
        }
    }

private:
    symbol_t n_branches_;
    trie_node<DIMENSION> *root_ = nullptr;
    level_t max_depth_;
    level_t trie_depth_;
    node_n_t initial_tree_capacity_;
    preorder_t max_tree_nodes_;
};

#endif //MD_TRIE_MD_TRIE_H
