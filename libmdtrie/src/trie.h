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
#include "compressed_bitmap.h"
#include <sys/stat.h>
#include <vector>
#include "defs.h"
#include "data_point.h"
#include "point_array.h"
#include "tree_block.h"
#include "trie_node.h"
// #include <mutex>
// std::mutex mutex;

template<dimension_t DIMENSION, symbol_t NUM_BRANCHES>
class md_trie {
public:
    explicit md_trie(level_t max_depth, level_t trie_depth,
                     preorder_t max_tree_nodes, uint8_t initial_capacity_nodes = 8) {
        // n_branches_ = (symbol_t) pow(2, DIMENSION);
        initial_tree_capacity_ = initial_capacity_nodes;
        max_depth_ = max_depth;
        trie_depth_ = trie_depth;
        max_tree_nodes_ = max_tree_nodes;
        root_ = new trie_node<DIMENSION, NUM_BRANCHES>(false);
    }

    inline trie_node<DIMENSION, NUM_BRANCHES> *root() {
        return root_;
    }



    // This function goes down the trie
    // Return the treeblock at the leaf of the trie
    tree_block<DIMENSION, NUM_BRANCHES> *walk_trie(trie_node<DIMENSION, NUM_BRANCHES> *current_trie_node, data_point<DIMENSION> *leaf_point, level_t &level) const {
        symbol_t current_symbol;
        while (level < trie_depth_ && current_trie_node->get_child(leaf_point->leaf_to_symbol(level, max_depth_)))
            current_trie_node = current_trie_node->get_child(leaf_point->leaf_to_symbol(level++, max_depth_));
        while (level < trie_depth_) {
            current_symbol = leaf_point->leaf_to_symbol(level, max_depth_);
            if (level == trie_depth_ - 1){
                current_trie_node->set_child(current_symbol, new trie_node<DIMENSION, NUM_BRANCHES>(true));
            }
            else
                current_trie_node->set_child(current_symbol, new trie_node<DIMENSION, NUM_BRANCHES>(false));
            current_trie_node->get_child(current_symbol)->parent_trie_node = current_trie_node;
            current_trie_node->get_child(current_symbol)->parent_symbol = current_symbol;
            current_trie_node = current_trie_node->get_child(current_symbol);
            level++;
        }
        tree_block<DIMENSION, NUM_BRANCHES> *current_treeblock = nullptr;
        if (current_trie_node->block() == nullptr) {
            current_treeblock = new tree_block<DIMENSION, NUM_BRANCHES>(trie_depth_, initial_tree_capacity_, 1, max_depth_, max_tree_nodes_, current_trie_node);
            current_trie_node->block(current_treeblock);
            // current_treeblock->set_parent_trie_node(current_trie_node);
        } else
            current_treeblock = (tree_block<DIMENSION, NUM_BRANCHES> *) current_trie_node->block();
        return current_treeblock;
    }

    // This function inserts a string into a trie_node.
    // The first part it traverses is the trie, followed by traversing the treeblock
    void insert_trie(data_point<DIMENSION> *leaf_point, level_t length) {

        // raise(SIGINT);
        // if (root_ == nullptr) {
        //     root_ = new trie_node<DIMENSION, NUM_BRANCHES>(false);
        // }
        level_t level = 0;
        trie_node<DIMENSION, NUM_BRANCHES> *current_trie_node = root_;
        tree_block<DIMENSION, NUM_BRANCHES> *current_treeblock = walk_trie(current_trie_node, leaf_point, level);

        // return;
        current_treeblock->insert_remaining(leaf_point, length, level);
        
    }

    // Used for Test script to check whether a leaf_point is present
    bool check(data_point<DIMENSION> *leaf_point, level_t strlen) const {
        level_t level = 0;
        trie_node<DIMENSION, NUM_BRANCHES> *current_trie_node = root_;
        tree_block<DIMENSION, NUM_BRANCHES> *current_treeblock = walk_trie(current_trie_node, leaf_point, level);
        return current_treeblock->walk_tree_block(leaf_point, strlen, level);
    }

    uint64_t size() const {

        uint64_t total_size = sizeof(trie_node<DIMENSION, NUM_BRANCHES> *) /*root_*/ + sizeof(level_t) * 2 +  sizeof(preorder_t) + sizeof(node_n_t);

        // Include primary key size:

        vector_size += sizeof(p_key_to_treeblock_compact) + (44 * total_points_count / 64 + 1) * 8;
        total_size += sizeof(p_key_to_treeblock_compact) + (44 * total_points_count / 64 + 1) * 8;

        // vector_size += sizeof(p_key_to_treeblock) + sizeof(uint64_t) * p_key_to_treeblock.size();
        // total_size += sizeof(p_key_to_treeblock) + sizeof(uint64_t) * p_key_to_treeblock.size();

        return total_size + root_->size();
    }

    void density(density_array *array) {
        root_->density(array);
    }

    void range_search_trie(data_point<DIMENSION> *start_range, data_point<DIMENSION> *end_range, trie_node<DIMENSION, NUM_BRANCHES> *current_trie_node,
                                    level_t level, point_array<DIMENSION> *found_points) {
        // If we reach the bottom of the top-level trie
        if (level == trie_depth_) {
            auto *current_treeblock = (tree_block<DIMENSION, NUM_BRANCHES> *) current_trie_node->block();
            current_treeblock->range_search_treeblock(start_range, end_range, current_treeblock, level, 0, 0, 0, 0, found_points);
            return;
        }
        symbol_t start_morton = start_range->leaf_to_symbol(level, max_depth_);
        symbol_t end_morton = end_range->leaf_to_symbol(level, max_depth_);
        symbol_t representation = start_morton ^ end_morton;
        symbol_t neg_representation = ~representation;

        struct data_point<DIMENSION> original_start_range = (*start_range);
        struct data_point<DIMENSION> original_end_range = (*end_range); 
        
        // for (symbol_t current_morton = 0; current_morton < n_branches_; current_morton++){

        // int count = 0;
        for (symbol_t current_morton = start_morton; current_morton <= end_morton; current_morton++){

            if ((start_morton & neg_representation) != (current_morton & neg_representation)){
                continue;
            }
            
            
            if (!current_trie_node->get_child(current_morton)) {
                continue;
            }
            // count ++;
            start_range->update_range_morton(end_range, current_morton, level, max_depth_);

            range_search_trie(start_range, end_range, current_trie_node->get_child(current_morton), level + 1,
                                found_points);

            (*start_range) = original_start_range;
            (*end_range) = original_end_range;                
        }

        // int trie_n_children = 0;

        // for (symbol_t i = 0; i < n_branches_; i++){
        //     if (current_trie_node->get_child(i)){
        //         trie_n_children++;
        //     }
        // }

        // if (trie_n_children != count){
        //     raise(SIGINT);
        // }
    }
    

private:
    // symbol_t n_branches_;
    trie_node<DIMENSION, NUM_BRANCHES> *root_ = nullptr;
    level_t max_depth_;
    level_t trie_depth_;
    node_n_t initial_tree_capacity_;
    preorder_t max_tree_nodes_;
};

#endif //MD_TRIE_MD_TRIE_H
