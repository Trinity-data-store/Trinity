#include "trie_node.h"
#include "tree_block.h"

uint64_t trie_node::size() const {
    uint64_t total_size = children_.size() * sizeof(trie_node *);
    if (!block_) {
        unsigned int children_size = children_.size(); 
        for (unsigned int i = 0; i < children_size; i++)
        // for (auto i : children_) {
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