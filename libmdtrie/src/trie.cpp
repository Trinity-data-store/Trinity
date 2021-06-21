#include "trie.h"

// Traverse the current TreeBlock, going into frontier nodes as needed
// Until it cannot traverse further and calls insertion
void md_trie::insert_remaining(tree_block *root, data_point *leaf_point, level_t length, level_t level) const {
    tree_block *current_block = root;
    node_t current_node = 0;
    preorder_t current_frontier = 0;

    node_t temp_node = 0;
    while (level < length) {
        temp_node = current_block->child(current_block, current_node,
                                         leaf_point->leaf_to_symbol(level, max_depth_), level,
                                         current_frontier);
        if (temp_node == (node_t) -1)
            break;
        current_node = temp_node;
        if (current_block->num_frontiers() > 0 && current_frontier < current_block->num_frontiers() &&
            current_node == current_block->get_preorder(current_frontier)) {
            current_block = current_block->get_pointer(current_frontier);
            current_node = (node_t) 0;
            current_frontier = 0;
        }
        level++;
    }
    current_block->insert(current_node, leaf_point, level, length, current_frontier);
}

// This function is used for testing.
// It differs from above as it only returns True or False.
bool
md_trie::walk_tree_block(tree_block *current_block, data_point *leaf_point, level_t length, level_t level) const {
    preorder_t current_frontier = 0;
    node_t current_node = 0;
    node_t temp_node = 0;
    while (level < length) {
        temp_node = current_block->child(current_block, current_node,
                                         leaf_point->leaf_to_symbol(level, max_depth_), level,
                                         current_frontier);
        if (temp_node == (node_t) -1)
            return false;
        current_node = temp_node;

        if (current_block->num_frontiers() > 0 && current_frontier < current_block->num_frontiers() &&
            current_node == current_block->get_preorder(current_frontier)) {
            current_block = current_block->get_pointer(current_frontier);
            current_node = (node_t) 0;
            current_frontier = 0;
        }
        level++;
    }
    return true;
}

// This function goes down the trie
// Return the treeblock at the leaf of the trie
tree_block *md_trie::walk_trie(trie_node *current_trie_node, data_point *leaf_point, level_t &level) const {
    symbol_t current_symbol;
    while (current_trie_node->get_child(leaf_point->leaf_to_symbol(level, max_depth_)))
        current_trie_node = current_trie_node->get_child(leaf_point->leaf_to_symbol(level++, max_depth_));
    while (level < trie_depth_) {
        current_symbol = leaf_point->leaf_to_symbol(level, max_depth_);
        current_trie_node->set_child(current_symbol, new trie_node(n_branches_));
        current_trie_node = current_trie_node->get_child(current_symbol);
        level++;
    }
    tree_block *current_treeblock = nullptr;
    if (current_trie_node->block() == nullptr) {
        current_treeblock = new tree_block(dimensions_, trie_depth_, initial_tree_capacity_, 1, max_depth_);
        current_trie_node->block(current_treeblock);
    } else
        current_treeblock = (tree_block *) current_trie_node->block();
    return current_treeblock;
}

// This function inserts a string into a trie_node.
// The first part it traverses is the trie, followed by traversing the treeblock
void md_trie::insert_trie(data_point *leaf_point, level_t length) {
    if (root_ == nullptr) {
        root_ = new trie_node(n_branches_);
    }
    level_t level = 0;
    trie_node *current_trie_node = root_;
    tree_block *current_treeblock = walk_trie(current_trie_node, leaf_point, level);
    insert_remaining(current_treeblock, leaf_point, length, level);
}

// Used for Test script to check whether a leaf_point is present
bool md_trie::check(data_point *leaf_point, level_t strlen) const {
    level_t level = 0;
    trie_node *current_trie_node = root_;
    tree_block *current_treeblock = walk_trie(current_trie_node, leaf_point, level);
    return walk_tree_block(current_treeblock, leaf_point, strlen, level);
}

uint64_t md_trie::size() const {
    uint64_t total_size = sizeof(uint8_t) + sizeof(symbol_t) + sizeof(trie_node *) + sizeof(level_t) * 2 +
                          sizeof(preorder_t) * 2;
    return total_size + root_->size();
}

void md_trie::density(density_array *array) {
    root_->density(array);
}

void md_trie::range_search_trie(data_point *start_range, data_point *end_range, trie_node *current_trie_node,
                                level_t level, point_array *found_points) {
    // If we reach the bottom of the top-level trie
    if (level == trie_depth_) {
        auto *current_treeblock = (tree_block *) current_trie_node->block();
        current_treeblock->range_search_treeblock(start_range, end_range, current_treeblock, level, 0, 0, found_points);
        return;
    }
    representation_t *representation = (representation_t *) malloc(sizeof(representation_t) * dimensions_);    
    start_range->get_representation(end_range, representation, level, max_depth_);
    range_traverse_trie(start_range, end_range, representation, 0, current_trie_node, level, found_points);
    free(representation);
}

void
md_trie::range_traverse_trie(data_point *start_range, data_point *end_range, representation_t representation[], uint8_t index,
                             trie_node *current_trie_node, level_t level, point_array *found_points) {
    if (index == dimensions_) {

        start_range->update_range(end_range, representation, level, max_depth_);

        symbol_t current_symbol = 0;
        for (dimension_t j = 0; j < dimensions_; j++) {
            current_symbol = current_symbol << 1U;
            if (representation[j] == 1) {
                current_symbol += 1;
            }
        }
        if (current_trie_node->get_child(current_symbol)) {
            range_search_trie(start_range, end_range, current_trie_node->get_child(current_symbol), level + 1,
                              found_points);
        }
        return;
    }
    if (representation[index] == 2) {
        data_point original_start_range(dimensions_);
        original_start_range.set(start_range->get());
        data_point original_end_range(dimensions_);
        original_end_range.set(end_range->get());  

        representation[index] = 0;
        range_traverse_trie(start_range, end_range, representation, index + 1, current_trie_node, level, found_points);

        start_range->set(original_start_range.get());
        end_range->set(original_end_range.get());
        
        representation[index] = 1;
        range_traverse_trie(start_range, end_range, representation, index + 1, current_trie_node, level, found_points);

        start_range->set(original_start_range.get());
        end_range->set(original_end_range.get());
        representation[index] = 2;  

        original_start_range.free_coordinates();
        original_end_range.free_coordinates();

    } else {
        range_traverse_trie(start_range, end_range, representation, index + 1, current_trie_node, level, found_points);
    }
}