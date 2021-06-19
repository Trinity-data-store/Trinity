#include "trie_node.h"
#include "tree_block.h"

uint64_t trie_node::size() const {
    symbol_t children_size = children_.size(); 
    uint64_t total_size = children_size * sizeof(trie_node *);
    if (!block_) {
        for (symbol_t i = 0; i < children_size; i++)
        {
            if (children_[i]) {
                total_size += children_[i]->size();
            }
        }
    } else {
        total_size += ((tree_block *) block_)->size();
    }
    return total_size;
}

void trie_node::density(density_array *array) {
    symbol_t children_size = children_.size(); 
    uint8_t current_node = 0;
    if (!block_) {
        for (symbol_t i = 0; i < children_size; i++)
        {
            if (children_[i]) {
                current_node += 1;
                children_[i]->density(array);
            }
        }
    } else {
        ((tree_block *) block_)->density(array);
    }
    array->push_back(current_node);
}
