#ifndef MD_TRIE_TRIE_NODE_H
#define MD_TRIE_TRIE_NODE_H

#include "defs.h"

class trie_node {
public:
    explicit trie_node(symbol_t num_branches) : children_(num_branches), block_(nullptr) {}

    trie_node *get_child(symbol_t symbol) {
        return children_[symbol];
    }

    void set_child(symbol_t symbol, trie_node *node) {
        children_[symbol] = node;
    }

    tree_block *block() const {
        return block_;
    }

    void block(tree_block *blk) {
        block_ = blk;
    }

    uint64_t size() const;

    void density(density_array *);

private:
    std::vector<trie_node *> children_;
    tree_block *block_;
};


#endif //MD_TRIE_TRIE_NODE_H
