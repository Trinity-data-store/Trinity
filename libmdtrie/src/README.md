### File Structure

trie.h defines the main function calls for the data structure, such as insert_trie and range_search_trie.  
tree_block.h defines the function calls for each treeblock, including insert, child and range_search_treeblock, select_subtree selects a node and its subtree to turn into a frontier node.  
This implementation includes a top-level pointer-based trie data structure, where each trie node, defined in trie_node.h, stores an array of pointers to children trie nodes. The size of this top-level trie could be adjusted. Its leaves point to child treeblocks.  
compressed_bitmap.h defines the structure of the bit vector in the treeblocks and supports collapsed nodes.  
compact_ptr.h implements how primary keys are stored at the leaf, either stored directly as value, in a vector, or a delta-encoded array (defined in delta_encoded_array.h)  