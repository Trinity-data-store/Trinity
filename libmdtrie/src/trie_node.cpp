#include "trie_node.h"
#include "tree_block.h"

uint64_t trie_node::size() const {
    uint64_t total_size = children_.size() * sizeof(trie_node *);
    if (!block_) {
        for (auto i : children_) {
            if (i) {
                total_size += i->size();
            }
        }
    } else {
        total_size += ((tree_block *) block_)->size();
    }
    return total_size;
}