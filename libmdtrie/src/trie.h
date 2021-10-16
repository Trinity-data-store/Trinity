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

class md_trie {
public:
    explicit md_trie(level_t max_depth, level_t trie_depth,
                     preorder_t max_tree_nodes, uint8_t initial_capacity_nodes = 1){ 
    
        initial_tree_capacity_ = initial_capacity_nodes;
        max_depth_ = max_depth;
        trie_depth_ = trie_depth;
        max_tree_nodes_ = max_tree_nodes;
        root_ = new trie_node(false, level_to_num_children[0]);
    }

    inline trie_node *root() {

        return root_;
    }

    tree_block *walk_trie(trie_node *current_trie_node, data_point *leaf_point, level_t &level) const {

        symbol_t current_symbol;

        while (level < trie_depth_ && current_trie_node->get_child(leaf_point->leaf_to_symbol(level))){
            
            current_trie_node = current_trie_node->get_child(leaf_point->leaf_to_symbol(level));
            level++;
        }
        while (level < trie_depth_) {

            current_symbol = leaf_point->leaf_to_symbol(level);
            if (level == trie_depth_ - 1){
                current_trie_node->set_child(current_symbol, new trie_node(true, level_to_num_children[level]));
            }
            else {
                current_trie_node->set_child(current_symbol, new trie_node(false, level_to_num_children[level]));
            }
            current_trie_node->get_child(current_symbol)->set_parent_trie_node(current_trie_node);
            current_trie_node->get_child(current_symbol)->set_parent_symbol(current_symbol);
            current_trie_node = current_trie_node->get_child(current_symbol);
            level++;
        }

        tree_block *current_treeblock = nullptr;
        if (current_trie_node->get_block() == nullptr) {
            current_treeblock = new tree_block(trie_depth_, initial_tree_capacity_ /*is 1*/, initial_tree_capacity_ * level_to_num_children[trie_depth_], 1, max_depth_, max_tree_nodes_, current_trie_node);
            current_trie_node->set_block(current_treeblock);
        } 
        else {
            current_treeblock = (tree_block *) current_trie_node->get_block();
        }

        return current_treeblock;
    }

    void insert_trie(data_point *leaf_point, n_leaves_t primary_key) {

        level_t level = 0;
        trie_node *current_trie_node = root_;
        tree_block *current_treeblock = walk_trie(current_trie_node, leaf_point, level);

        current_treeblock->insert_remaining(leaf_point, level, primary_key);
    }


    bool check(data_point *leaf_point) const {

        level_t level = 0;
        trie_node *current_trie_node = root_;
        tree_block *current_treeblock = walk_trie(current_trie_node, leaf_point, level);

        return current_treeblock->walk_tree_block(leaf_point, level);
    }

    uint64_t size() const {

        // TODO
        return 0;
    }


    void range_search_trie(data_point *start_range, data_point *end_range, trie_node *current_trie_node,
                                    level_t level, point_array *found_points) {

        if (level == trie_depth_) {

            auto *current_treeblock = (tree_block *) current_trie_node->get_block();
            current_treeblock->range_search_treeblock(start_range, end_range, current_treeblock, level, 0, 0, 0, 0, 0, 0, found_points);
            return;
        }
        
        symbol_t start_symbol = start_range->leaf_to_symbol(level);
        symbol_t end_symbol = end_range->leaf_to_symbol(level);
        symbol_t representation = start_symbol ^ end_symbol;
        symbol_t neg_representation = ~representation;

        struct data_point original_start_range = (*start_range);
        struct data_point original_end_range = (*end_range); 

        for (symbol_t current_symbol = start_symbol; current_symbol <= end_symbol; current_symbol++){

            if ((start_symbol & neg_representation) != (current_symbol & neg_representation)){
                continue;
            }            
            
            if (!current_trie_node->get_child(current_symbol)) {
                continue;
            }

            start_range->update_symbol(end_range, current_symbol, level);

            range_search_trie(start_range, end_range, current_trie_node->get_child(current_symbol), level + 1,
                                found_points);

            (*start_range) = original_start_range;
            (*end_range) = original_end_range;                
        }

    }
    

private:
    trie_node *root_ = nullptr;
    level_t max_depth_;
    level_t trie_depth_;
    node_n_t initial_tree_capacity_;
    preorder_t max_tree_nodes_;
};

#endif //MD_TRIE_MD_TRIE_H
