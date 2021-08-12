#ifndef MD_TRIE_BIMAP_H
#define MD_TRIE_BIMAP_H


#include <boost/bimap.hpp>

typedef uint64_t preorder_t;
typedef uint64_t symbol_t;
typedef uint8_t dimension_t;

template<dimension_t DIMENSION>
class tree_block;

// typedef std::tuple<void *, preorder_t, symbol_t> node_triple;
// typedef boost::bimap<preorder_t, node_triple> Bimap;
// typedef boost::bimap<preorder_t, preorder_t> Bimap;
// Bimap primary_tuple_map;

std::unordered_map<preorder_t, preorder_t> p_key_to_node;
std::unordered_map<preorder_t, preorder_t> node_to_p_key;
preorder_t map_size = 0;

preorder_t generate_primary_key()
{
    map_size ++;
    return map_size;
}

void insert_bimap(preorder_t parent_node)
{
    preorder_t p_key = generate_primary_key();
    p_key_to_node[p_key] = parent_node;
    node_to_p_key[parent_node] = p_key;
}


#endif //MD_TRIE_BIMAP_H

