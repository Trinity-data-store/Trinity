#ifndef MD_TRIE_TRIE_NODE_H
#define MD_TRIE_TRIE_NODE_H

#include "defs.h"
#include <cstdlib>
#include "tree_block.h"
#include <mutex>
#include <shared_mutex>

template<dimension_t DIMENSION, symbol_t NUM_BRANCHES>
class trie_node {
public:

    trie_node *parent_trie_node = NULL; // Most cases set
    symbol_t parent_symbol = 0; // Most cases set

    explicit trie_node(bool is_leaf) {
        
        is_leaf_ = is_leaf;
        if (!is_leaf){
            trie_or_treeblock_ptr_ = (trie_node<DIMENSION, NUM_BRANCHES> **)calloc(sizeof(trie_node<DIMENSION, NUM_BRANCHES> *), NUM_BRANCHES);
        }
    }

    inline trie_node<DIMENSION, NUM_BRANCHES> *get_child(symbol_t symbol) {

        return ((trie_node<DIMENSION, NUM_BRANCHES> **)trie_or_treeblock_ptr_)[symbol];
    }

    inline void set_child(symbol_t symbol, trie_node *node) {
        mutex_.lock();
        ((trie_node<DIMENSION, NUM_BRANCHES> **)trie_or_treeblock_ptr_)[symbol] = node;
        mutex_.unlock();
    }

    inline tree_block<DIMENSION, NUM_BRANCHES> *block() const {

        return (tree_block<DIMENSION, NUM_BRANCHES> *)trie_or_treeblock_ptr_;
    }

    inline void block(tree_block<DIMENSION, NUM_BRANCHES> *blk) {
        mutex_.lock();
        trie_or_treeblock_ptr_ = blk;
        mutex_.unlock();
    }

    uint64_t size() const {
        
        // Array of Trie node pointers
        trie_size += NUM_BRANCHES * sizeof(trie_node *) + sizeof(trie_or_treeblock_ptr_) /*+ sizeof(is_leaf_)*/; 
        uint64_t total_size = NUM_BRANCHES * sizeof(trie_node *) + sizeof(trie_or_treeblock_ptr_) /*+ sizeof(is_leaf_)*/; 

        // parent trie node + parent trie symbol
        trie_size += sizeof(trie_node *) + sizeof(uint16_t) /*symbol_t*/; 
        total_size += sizeof(trie_node *) + sizeof(uint16_t) /*symbol_t*/; 

        // Treeblock pointer for bottom level trie
        // total_size += sizeof(tree_block<DIMENSION, NUM_BRANCHES> *); 
        if (!is_leaf_) {
            for (symbol_t i = 0; i < NUM_BRANCHES; i++)
            {
                if (((trie_node<DIMENSION, NUM_BRANCHES> **)trie_or_treeblock_ptr_)[i]) {
                    total_size += ((trie_node<DIMENSION, NUM_BRANCHES> **)trie_or_treeblock_ptr_)[i]->size();
                }
            }
        } else {
            total_size += ((tree_block<DIMENSION, NUM_BRANCHES> *) trie_or_treeblock_ptr_)->size();
        }
        return total_size;
    }

    void density(density_array *array) {
        uint8_t current_node = 0;
        if (!is_leaf_) {
            for (symbol_t i = 0; i < NUM_BRANCHES; i++)
            {
                if (((trie_node<DIMENSION, NUM_BRANCHES> **)trie_or_treeblock_ptr_)[i]) {
                    current_node += 1;
                    ((trie_node<DIMENSION, NUM_BRANCHES> **)trie_or_treeblock_ptr_)[i]->density(array);
                }
            }
        } else {
            ((tree_block<DIMENSION, NUM_BRANCHES> *) trie_or_treeblock_ptr_)->density(array);
            return;
        }
        (*array)[current_node] = (*array)[current_node] + 1;
    }

    void get_node_path_from_treeblock(level_t level, symbol_t *node_path){

        get_node_path(level, node_path);
    }

    void get_node_path(level_t level, symbol_t *node_path)
    {
        if (parent_trie_node){
            node_path[level - 1] = parent_symbol;
            parent_trie_node->get_node_path(level - 1, node_path);
        }
    }

private:

    bool is_leaf_ = false;
    void *trie_or_treeblock_ptr_ = NULL;
    std::shared_mutex mutex_;
};

#endif //MD_TRIE_TRIE_NODE_H
