#ifndef MD_TRIE_TRIE_NODE_H
#define MD_TRIE_TRIE_NODE_H

#include "defs.h"
#include <cstdlib>

class trie_node {
public:
    explicit trie_node(symbol_t num_branches) : block_(nullptr) {
        children_ = (trie_node **)calloc(num_branches, sizeof(trie_node *));
        size_ = num_branches;
    }

    inline trie_node *get_child(symbol_t symbol) {
        return children_[symbol];
    }

    inline void set_child(symbol_t symbol, trie_node *node) {
        children_[symbol] = node;
    }

    inline tree_block *block() const {
        return block_;
    }

    inline void block(tree_block *blk) {
        block_ = blk;
    }

    uint64_t size() const;

    void density(density_array *);

private:
    trie_node **children_;
    // std::vector<trie_node *> children_;
    symbol_t size_;
    tree_block *block_;
};


#endif //MD_TRIE_TRIE_NODE_H
