#ifndef MD_TRIE_TRIE_NODE_H
#define MD_TRIE_TRIE_NODE_H

#include "defs.h"
#include <cstdlib>
#include "tree_block.h"

template<dimension_t DIMENSION>
class trie_node {
public:

    trie_node *parent_trie_node = NULL;
    symbol_t parent_symbol = 0;

    explicit trie_node(symbol_t num_branches) : block_(nullptr) {
        children_ = (trie_node<DIMENSION> **)calloc(num_branches, sizeof(trie_node<DIMENSION> *));
        size_ = num_branches;
    }

    inline trie_node<DIMENSION> *get_child(symbol_t symbol) {
        return children_[symbol];
    }

    inline void set_child(symbol_t symbol, trie_node *node) {
        children_[symbol] = node;
    }

    inline tree_block<DIMENSION> *block() const {
        return block_;
    }

    inline void block(tree_block<DIMENSION> *blk) {
        block_ = blk;
    }

    uint64_t size() const {
        uint64_t total_size = size_ * sizeof(trie_node *) + sizeof(trie_node *);
        if (parent_trie_node){
            total_size += sizeof(trie_node *);
        }

        if (!block_) {
            for (symbol_t i = 0; i < size_; i++)
            {
                if (children_[i]) {
                    total_size += children_[i]->size();
                }
            }
        } else {
            total_size += ((tree_block<DIMENSION> *) block_)->size();
        }
        return total_size;
    }

    void density(density_array *array) {
        uint8_t current_node = 0;
        if (!block_) {
            for (symbol_t i = 0; i < size_; i++)
            {
                if (children_[i]) {
                    current_node += 1;
                    children_[i]->density(array);
                }
            }
        } else {
            ((tree_block<DIMENSION> *) block_)->density(array);
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
    trie_node<DIMENSION> **children_;
    symbol_t size_;
    tree_block<DIMENSION> *block_;
};


#endif //MD_TRIE_TRIE_NODE_H
