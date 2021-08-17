#ifndef MD_TRIE_BIMAP_H
#define MD_TRIE_BIMAP_H

#include <boost/bimap.hpp>
#include <sys/time.h>

typedef uint64_t preorder_t;
typedef uint64_t symbol_t;
typedef uint8_t dimension_t;

struct node_config {

    uint64_t parent_ptr;
    unsigned int parent_node : 12;
    unsigned int symbol : 12;

    // Required for bimap to work
    bool operator==(const node_config &o) const {
        return parent_ptr == o.parent_ptr && parent_node == o.parent_node && symbol == o.symbol;
    }
};

// Reference: https://dawnarc.com/2019/09/c-how-to-use-a-struct-as-key-in-a-std-map/

namespace std {

template <>
struct hash<node_config>
{
    std::size_t operator()(const node_config& n) const
    {
        using std::size_t;
        using std::hash;
        using std::uint64_t;

        // Compute individual hash values for first,
        // second and third and combine them using XOR
        // and bit shifting:

        return ((hash<uint64_t>()(n.parent_ptr)
        ^ (hash<uint64_t>()(n.parent_node) << 1)) >> 1)
        ^ (hash<uint64_t>()(n.symbol) << 1);
        }
    };
}

std::unordered_map<preorder_t, node_config> p_key_to_node;
std::unordered_map<node_config, preorder_t> node_to_p_key;

preorder_t map_size = 0;

typedef unsigned long long int TimeStamp;
static TimeStamp GetTimestamp();

TimeStamp total_bimap = 0;
uint64_t bimap_insertion_count = 0;

void insert_bimap(void *treeblock_ptr, preorder_t parent_node, symbol_t symbol)
{
    TimeStamp start = GetTimestamp(); 
    map_size++;
    bimap_insertion_count++;

    node_config node;
    node.parent_ptr = (uint64_t) treeblock_ptr;
    node.parent_node = parent_node;
    node.symbol = symbol;
    
    p_key_to_node.emplace(map_size, node);
    node_to_p_key.emplace(node, map_size);

    total_bimap += GetTimestamp() - start;
}

bool check_bimap_node_config(void *treeblock_ptr, preorder_t parent_node, symbol_t symbol)
{

    std::unordered_map<node_config, preorder_t>::const_iterator node_to_p_key_itr;  

    node_config node;
    node.parent_ptr = (uint64_t) treeblock_ptr;
    node.parent_node = parent_node;
    node.symbol = symbol;
    
    node_to_p_key_itr = node_to_p_key.find(node);
    return node_to_p_key_itr != node_to_p_key.end();

}

#endif //MD_TRIE_BIMAP_H

