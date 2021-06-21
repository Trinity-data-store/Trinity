#include "trie_node.h"
#include "tree_block.h"

uint64_t trie_node::size() const {
    uint64_t total_size = size_ * sizeof(trie_node *);
    if (!block_) {
        for (symbol_t i = 0; i < size_; i++)
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
        ((tree_block *) block_)->density(array);
        return;
    }
    (*array)[current_node] = (*array)[current_node] + 1;
    // array->push_back(current_node);
}
