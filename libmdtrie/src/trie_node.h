#ifndef MD_TRIE_TRIE_NODE_H
#define MD_TRIE_TRIE_NODE_H

#include "defs.h"
#include <cstdlib>
#include "tree_block.h"
#include <sys/mman.h>

template<dimension_t DIMENSION>
class trie_node {
    
public:

    explicit trie_node(bool is_leaf, dimension_t num_dimensions) {
        
        is_leaf_ = is_leaf;
        if (!is_leaf){

            trie_or_treeblock_ptr_ = (trie_node<DIMENSION> **)calloc(sizeof(trie_node<DIMENSION> *), 1 << num_dimensions);
        }
        num_children_ = 1 << num_dimensions;
    }

    inline trie_node<DIMENSION> *get_child(morton_t symbol) {

        auto trie_ptr = (trie_node<DIMENSION> **) trie_or_treeblock_ptr_;
        return trie_ptr[symbol];
    }

    inline void set_child(morton_t symbol, trie_node *node) {

        auto trie_ptr = (trie_node<DIMENSION> **) trie_or_treeblock_ptr_;
        trie_ptr[symbol] = node;
    }

    inline tree_block<DIMENSION> *get_block() const {
        if (!is_leaf_)
            return nullptr;
        return (tree_block<DIMENSION> *)trie_or_treeblock_ptr_;
    }

    inline void set_block(tree_block<DIMENSION> *block) {

        trie_or_treeblock_ptr_ = block;
        is_leaf_ = true;
    }

    uint64_t size() {

        uint64_t total_size = sizeof(uint64_t); // trie_or_treeblock_ptr_
        total_size += sizeof(uint64_t); // parent_trie_node_
        total_size += sizeof(uint8_t); // parent_symbol_

        if (!is_leaf_)
            total_size += sizeof(trie_node<DIMENSION> *) * num_children_; // Array of Trie node ptrs

        // is_leaf and num_children_ can be deduced from outside
        return total_size;
    }

    void get_node_path(level_t level, morton_t *node_path){

        if (parent_trie_node_){
            node_path[level - 1] = parent_symbol_;
            parent_trie_node_->get_node_path(level - 1, node_path);
        }
    }

    trie_node<DIMENSION> *get_parent_trie_node(){

        return parent_trie_node_;
    }

    morton_t get_parent_symbol(){
        return parent_symbol_;
    }

    void set_parent_trie_node(trie_node<DIMENSION> *node){

        parent_trie_node_ = node;
    }

    void set_parent_symbol(morton_t symbol){

        parent_symbol_ = symbol;
    }

    bool is_leaf(){
        return is_leaf_;
    }

    dimension_t get_num_children(){
        return num_children_;
    }

private:

    bool is_leaf_ = false;
    void *trie_or_treeblock_ptr_ = NULL;
    trie_node<DIMENSION> *parent_trie_node_ = NULL; 
    morton_t parent_symbol_ = 0; 
    dimension_t num_children_ = 0;
};

#endif //MD_TRIE_TRIE_NODE_H
