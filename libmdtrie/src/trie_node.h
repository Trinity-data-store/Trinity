#ifndef MD_TRIE_TRIE_NODE_H
#define MD_TRIE_TRIE_NODE_H

#include "defs.h"
#include <cstdlib>
#include "tree_block.h"

class trie_node {
    
public:

    explicit trie_node(bool is_leaf, dimension_t num_children) {
        
        is_leaf_ = is_leaf;
        if (!is_leaf){
            trie_or_treeblock_ptr_ = new std::vector<trie_node *>(num_children, 0);
        }
    }

    inline trie_node *get_child(symbol_t symbol) {

        auto trie_ptr = (std::vector<trie_node *> *) trie_or_treeblock_ptr_;

        return (* trie_ptr)[symbol];
    }

    inline void set_child(symbol_t symbol, trie_node *node) {
        
        auto trie_ptr = (std::vector<trie_node *> *) trie_or_treeblock_ptr_;
        (* trie_ptr)[symbol] = node;
    }

    inline tree_block *get_block() const {

        return (tree_block *)trie_or_treeblock_ptr_;
    }

    inline void set_block(tree_block *block) {

        trie_or_treeblock_ptr_ = block;
    }

    uint64_t size() const {

        // TODO
        return 0;
    }

    void get_node_path(level_t level, symbol_t *node_path){

        if (parent_trie_node_){
            node_path[level - 1] = parent_symbol_;
            parent_trie_node_->get_node_path(level - 1, node_path);
        }
    }

    trie_node *get_parent_trie_node(){
        return parent_trie_node_;
    }

    symbol_t get_parent_symbol(){

        return parent_symbol_;
    }

    void set_parent_trie_node(trie_node *node){
        parent_trie_node_ = node;
    }

    void set_parent_symbol(symbol_t symbol){
        parent_symbol_ = symbol;
    }

private:

    bool is_leaf_ = false;
    void *trie_or_treeblock_ptr_ = NULL;
    trie_node *parent_trie_node_ = NULL; 
    symbol_t parent_symbol_ = 0; 
};

#endif //MD_TRIE_TRIE_NODE_H
