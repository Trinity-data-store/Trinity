#ifndef MD_TRIE_TRIE_NODE_H
#define MD_TRIE_TRIE_NODE_H

#include "defs.h"
#include <cstdlib>
#include "tree_block.h"
#include <sys/mman.h>

class trie_node {
    
public:

    explicit trie_node(bool is_leaf, dimension_t num_dimensions) {
        
        is_leaf_ = is_leaf;
        if (!is_leaf){

            // trie_or_treeblock_ptr_ = (trie_node **)calloc(sizeof(trie_node *), 1 << num_dimensions);

            trie_or_treeblock_ptr_ = (trie_node **) mmap( NULL, (1 << num_dimensions)* sizeof(trie_node *),
                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

            // trie_or_treeblock_ptr_ = (trie_node **) mmap( NULL, (1 << num_dimensions) * sizeof(trie_node *),
                // PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, offset);
            // offset = (offset + (1 << num_dimensions) * sizeof(trie_node *)) & ~(sysconf(_SC_PAGE_SIZE) - 1);
        }
        num_children_ = 1 << num_dimensions;
    }

    inline trie_node *get_child(symbol_t symbol) {

        auto trie_ptr = (trie_node **) trie_or_treeblock_ptr_;
        return trie_ptr[symbol];
    }

    inline void set_child(symbol_t symbol, trie_node *node) {

        auto trie_ptr = (trie_node **) trie_or_treeblock_ptr_;
        trie_ptr[symbol] = node;
    }

    inline tree_block *get_block() const {
        if (!is_leaf_)
            return nullptr;
        return (tree_block *)trie_or_treeblock_ptr_;
    }

    inline void set_block(tree_block *block) {

        trie_or_treeblock_ptr_ = block;
        is_leaf_ = true;
    }

    uint64_t size() {

        // Array of Trie node pointers
        uint64_t total_size = sizeof(trie_or_treeblock_ptr_);
        total_size += sizeof(trie_node *) + sizeof(uint16_t) /*symbol_t*/; 
        total_size += sizeof(trie_node *) * num_children_;

        return total_size;
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

    bool is_leaf(){
        return is_leaf_;
    }

    dimension_t get_num_children(){
        return num_children_;
    }

    virtual size_t Serialize(std::ostream& out) {

        size_t out_size = 0;

        // bool is_leaf_;
        out.write(reinterpret_cast<const char *>(&is_leaf_), sizeof(bool));
        out_size += sizeof(bool);         

        // void *trie_or_treeblock_ptr_ = NULL;
        out.write(reinterpret_cast<const char *>(&trie_or_treeblock_ptr_), sizeof(uint64_t));
        out_size += sizeof(uint64_t);

        // trie_node *parent_trie_node_;
        out.write(reinterpret_cast<const char *>(&parent_trie_node_), sizeof(trie_node *));
        out_size += sizeof(trie_node *);

        // symbol_t parent_symbol_ = 0; 
        out.write(reinterpret_cast<const char *>(&parent_symbol_), sizeof(symbol_t));
        out_size += sizeof(symbol_t);           

        // dimension_t num_children_ = 0;
        out.write(reinterpret_cast<const char *>(&num_children_), sizeof(dimension_t));
        out_size += sizeof(dimension_t);          

        return out_size;
    } 

    virtual size_t Deserialize(std::istream& in) {

        size_t in_size = 0;

        // bool is_leaf_;
        in.read(reinterpret_cast<char *>(&is_leaf_), sizeof(bool));
        in_size += sizeof(bool);         

        // void *trie_or_treeblock_ptr_ = NULL;
        in.read(reinterpret_cast<char *>(&trie_or_treeblock_ptr_), sizeof(uint64_t));
        in_size += sizeof(uint64_t);

        // trie_node *parent_trie_node_;
        in.read(reinterpret_cast<char *>(&parent_trie_node_), sizeof(trie_node *));
        in_size += sizeof(trie_node *);

        // symbol_t parent_symbol_ = 0; 
        in.read(reinterpret_cast<char *>(&parent_symbol_), sizeof(symbol_t));
        in_size += sizeof(symbol_t);           

        // dimension_t num_children_ = 0;
        in.read(reinterpret_cast<char *>(&num_children_), sizeof(dimension_t));
        in_size += sizeof(dimension_t);          

        return in_size;
    } 


private:

    bool is_leaf_ = false;
    void *trie_or_treeblock_ptr_ = NULL;
    trie_node *parent_trie_node_ = NULL; 
    symbol_t parent_symbol_ = 0; 
    dimension_t num_children_ = 0;
};

#endif //MD_TRIE_TRIE_NODE_H
