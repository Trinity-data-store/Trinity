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

template<dimension_t DIMENSION>
class md_trie {
public:
    explicit md_trie(level_t max_depth = 10, level_t trie_depth = 3,
                     preorder_t max_tree_nodes = 256, uint8_t initial_capacity_nodes = 2) {
        n_branches_ = (symbol_t) pow(2, DIMENSION);
        initial_tree_capacity_ = n_branches_ * initial_capacity_nodes;
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
            current_trie_node = current_trie_node->get_child(current_symbol);
            level++;
        }
        tree_block<DIMENSION> *current_treeblock = nullptr;
        if (current_trie_node->block() == nullptr) {
            current_treeblock = new tree_block<DIMENSION>(trie_depth_, initial_tree_capacity_, 1, max_depth_);
            current_trie_node->block(current_treeblock);
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
            current_treeblock->range_search_treeblock(start_range, end_range, current_treeblock, level, 0, 0, found_points);
            return;
        }
        representation_t representation[DIMENSION];
        start_range->get_representation(end_range, representation, level, max_depth_);
        range_traverse_trie(start_range, end_range, representation, 0, current_trie_node, level, found_points);

        // uint8_t index
        // dimension_t backtrack_size = 0;
        // representation_t backtrack_array[DIMENSIONS];

        // while (index <= DIMENSION){
        //     if (representation[index] == 1 || representation[index] == 0){
        //         index += 1;
        //     }
        //     else if (representation[index] == 2){
        //         backtrack_array[backtrack_size] = index;
        //         backtrack_size += 1;
        //         index += 1;
        //         representation[index] = 0;
        //     }
        //     if (index == DIMENSION){
        //         start_range->update_range(end_range, representation, level, max_depth_);

        //         symbol_t current_symbol = 0;
        //         for (dimension_t j = 0; j < DIMENSION; j++) {
        //             current_symbol = current_symbol << 1U;
        //             if (representation[j] == 1) {
        //                 current_symbol += 1;
        //             }
        //         }
        //         if (current_trie_node->get_child(current_symbol)) {
        //             range_search_trie(start_range, end_range, current_trie_node->get_child(current_symbol), level + 1,
        //                             found_points);
        //         }
        //         if (backtrack_size > 0){
        //             dimension_t index_previous = backtrack_array[backtrack_size - 1];
        //             backtrack_size --;
        //             representation[index_previous] = 1;
        //             index = index_previous + 1;
        //         }
        //         else {
        //             break;
        //         }

        //     }

            
        // }

    }

    void range_traverse_trie(data_point<DIMENSION> *start_range, data_point<DIMENSION> *end_range, representation_t representation[], uint8_t index,
                                trie_node<DIMENSION> *current_trie_node, level_t level, point_array<DIMENSION> *found_points) {
        if (index == DIMENSION) {

            start_range->update_range(end_range, representation, level, max_depth_);

            symbol_t current_symbol = 0;
            for (dimension_t j = 0; j < DIMENSION; j++) {
                current_symbol = current_symbol << 1U;
                if (representation[j] == 1) {
                    current_symbol += 1;
                }
            }
            if (current_trie_node->get_child(current_symbol)) {
                range_search_trie(start_range, end_range, current_trie_node->get_child(current_symbol), level + 1,
                                found_points);
            }
            return;
        }
        if (representation[index] == 2) {
            struct data_point<DIMENSION> original_start_range = (*start_range);
            struct data_point<DIMENSION> original_end_range = (*end_range); 

            representation[index] = 0;
            range_traverse_trie(start_range, end_range, representation, index + 1, current_trie_node, level, found_points);

            (*start_range) = original_start_range;
            (*end_range) = original_end_range;
            
            representation[index] = 1;
            range_traverse_trie(start_range, end_range, representation, index + 1, current_trie_node, level, found_points);

            (*start_range) = original_start_range;
            (*end_range) = original_end_range;
            representation[index] = 2;  

        } else {
            range_traverse_trie(start_range, end_range, representation, index + 1, current_trie_node, level, found_points);
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
