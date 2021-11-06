#ifndef MD_TRIE_TREE_BLOCK_H
#define MD_TRIE_TREE_BLOCK_H

#include "point_array.h"
#include "trie_node.h"
#include "compressed_bitmap.h"
#include <sys/time.h>
#include <cmath>
#include "compact_ptr.h"

class tree_block {
public:

    explicit tree_block(level_t root_depth, node_n_t node_capacity, node_n_t bit_capacity, node_n_t num_nodes,
                        level_t max_depth, node_n_t max_tree_nodes, trie_node *parent_trie_node) {

        root_depth_ = root_depth;
        node_capacity_ = node_capacity;
        max_depth_ = max_depth;
        max_tree_nodes_ = max_tree_nodes;
        num_nodes_ = num_nodes;
        bit_capacity_ = bit_capacity;
        total_nodes_bits_ = bit_capacity;
        dfuds_ = new compressed_bitmap::compressed_bitmap(node_capacity, bit_capacity);
        parent_trie_node_ = parent_trie_node;
    }

    inline node_n_t num_frontiers() {

        return num_frontiers_;
    }

    inline tree_block *get_pointer(preorder_t current_frontier) {

        return frontiers_[current_frontier].pointer_;
    }

    inline preorder_t get_preorder(preorder_t current_frontier)  
    {
        return frontiers_[current_frontier].preorder_;
    }

    inline void set_preorder(preorder_t current_frontier, preorder_t preorder) 
    {
        frontiers_[current_frontier].preorder_ = preorder;
    }

    inline void set_pointer(preorder_t current_frontier, tree_block *pointer) 
    {
        frontiers_[current_frontier].pointer_ = pointer;
        pointer->parent_tree_block_ = this;
        pointer->treeblock_frontier_num_ = get_preorder(current_frontier);
    }

    node_t select_subtree(preorder_t &subtree_size, preorder_t &selected_node_depth, preorder_t &selected_node_pos, preorder_t &num_primary, preorder_t &selected_primary_index, preorder_t *node_to_primary, preorder_t *node_to_depth) {

        // index -> Number of children & preorder
        node_info index_to_node[4096];

        // index -> size of subtree & preorder
        subtree_info index_to_subtree[4096];
        num_primary = 0;

        // Index -> depth of the node
        preorder_t index_to_depth[4096];

        //  Corresponds to index_to_node, index_to_subtree, index_to_depth
        preorder_t node_stack_top = 0, subtree_stack_top = 0, depth_stack_top = 0;
        preorder_t current_frontier = 0;
        selected_primary_index = 0;

        node_n_t current_node_pos = 0;
        index_to_node[node_stack_top].preorder_ = 0;
        index_to_node[node_stack_top].n_children_ = dfuds_->get_num_children(0, 0, level_to_num_children[root_depth_]);

        node_stack_top++;
        node_to_depth[0] = root_depth_;
        preorder_t prev_depth = root_depth_;
        preorder_t depth = root_depth_ + 1;
        preorder_t next_frontier_preorder;

        if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
            next_frontier_preorder = -1;
        else
            next_frontier_preorder = get_preorder(current_frontier);

        for (preorder_t i = 1; i < num_nodes_; i++) {

            current_node_pos += dfuds_->get_num_bits(i - 1, prev_depth);
            prev_depth = depth;

            node_to_depth[i] = depth;
            if (depth == max_depth_ - 1){

                node_to_primary[i] = dfuds_->get_num_children(i, current_node_pos, level_to_num_children[depth]);
            }
            if (i == next_frontier_preorder) {

                current_frontier++;
                if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
                    next_frontier_preorder = -1;
                else
                    next_frontier_preorder = get_preorder(current_frontier);
                index_to_node[node_stack_top - 1].n_children_--;
            }
            //  Start searching for its children
            else if (depth < max_depth_ - 1) {

                index_to_node[node_stack_top].preorder_ = i;
                index_to_node[node_stack_top++].n_children_ = dfuds_->get_num_children(i, current_node_pos, level_to_num_children[depth]);
                depth++;
            }
            //  Reached the maxDepth level
            else{
                index_to_node[node_stack_top - 1].n_children_--;
            }
                
            while (node_stack_top > 0 && index_to_node[node_stack_top - 1].n_children_ == 0) {

                index_to_subtree[subtree_stack_top].preorder_ = index_to_node[node_stack_top - 1].preorder_;
                index_to_subtree[subtree_stack_top].subtree_size_ = i - index_to_node[node_stack_top - 1].preorder_ + 1;
                subtree_stack_top++;
                node_stack_top--;
                index_to_depth[depth_stack_top] = depth;
                depth_stack_top++;
                depth--;
                if (node_stack_top == 0)
                    break;
                else
                    index_to_node[node_stack_top - 1].n_children_--;
            }
        }
        current_node_pos += dfuds_->get_num_bits(num_nodes_ - 1, prev_depth);

        // Go through the index_to_subtree vector to choose the proper subtree
        preorder_t min_node = 0;
        preorder_t min = (preorder_t) -1;
        preorder_t min_index = 0;
        preorder_t diff = (preorder_t) -1;
        auto leftmost = (preorder_t) -1;

        for (preorder_t i = 0; i < subtree_stack_top; i++) {
            auto subtree_size_at_i = (preorder_t) index_to_subtree[i].subtree_size_;
            if (index_to_subtree[i].preorder_ != 0 && num_nodes_ <= subtree_size_at_i * 4 && subtree_size_at_i * 4 <= 3 * num_nodes_ &&
                index_to_subtree[i].preorder_ < leftmost) {

                leftmost = min_node = index_to_subtree[i].preorder_;
                min_index = i;
            }
        }

        if (leftmost == (preorder_t) -1) {
            min_node = index_to_subtree[1].preorder_;
            if (num_nodes_ > 2 * index_to_subtree[1].subtree_size_) {
                min = num_nodes_ - 2 * index_to_subtree[1].subtree_size_;
            } else {
                min = 2 * index_to_subtree[1].subtree_size_ - num_nodes_;
            }
            min_index = 1;

            for (preorder_t i = 1; i < subtree_stack_top; i++) {
                if (num_nodes_ > 2 * index_to_subtree[i].subtree_size_) {
                    diff = num_nodes_ - 2 * index_to_subtree[i].subtree_size_;
                } else {
                    diff = 2 * index_to_subtree[i].subtree_size_ - num_nodes_;
                }
                if (diff < min) {
                    min = diff;
                    min_node = index_to_subtree[i].preorder_;
                    min_index = i;
                }
            }
        }
        subtree_size = index_to_subtree[min_index].subtree_size_;
        selected_node_depth = index_to_depth[min_index];
        
        for (preorder_t i = 0; i < min_node; i++ ){
            selected_primary_index += node_to_primary[i];
        }
        for (preorder_t i = 1; i <= min_node; i++){
            selected_node_pos += dfuds_->get_num_bits(i - 1, node_to_depth[i - 1]);
        }
        for (preorder_t i = min_node; i < min_node + subtree_size; i ++){
            num_primary += node_to_primary[i];
        }
        selected_node_depth = node_to_depth[min_node];

        return min_node;
    }

    // This function takes in a node (in preorder) and a symbol (branch index)
    // Return the child node (in preorder) designated by that symbol
    node_t skip_children_subtree(node_t node, preorder_t &node_pos, symbol_t symbol, level_t current_level,
                                            preorder_t &current_frontier, preorder_t &current_primary)  {

        if (current_level == max_depth_){
            return node;
        }

        int sTop = -1;
        preorder_t n_children_skip = dfuds_->get_child_skip(node, node_pos, symbol, level_to_num_children[current_level]);
        preorder_t n_children = dfuds_->get_num_children(node, node_pos, level_to_num_children[current_level]);
        preorder_t diff = n_children - n_children_skip;
        preorder_t stack[100];
        sTop++;
        stack[sTop] = n_children;

        preorder_t current_node_pos = node_pos + dfuds_->get_num_bits(node, current_level); //TODO
        node_t current_node = node + 1;

        if (frontiers_ != nullptr && current_frontier < num_frontiers_ && current_node > get_preorder(current_frontier))
            ++current_frontier;
        preorder_t next_frontier_preorder;

        if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
            next_frontier_preorder = -1;
        else
            next_frontier_preorder = get_preorder(current_frontier);

        current_level++;
        while (current_node < num_nodes_ && sTop >= 0 && diff < stack[0]) {
            if (current_node == next_frontier_preorder) {
                current_frontier++;
                if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
                    next_frontier_preorder = -1;
                else
                    next_frontier_preorder = get_preorder(current_frontier);
                stack[sTop]--;

                current_node_pos += dfuds_->get_num_bits(current_node, current_level);
            }
            // It is "-1" because current_level is 0th indexed.
            else if (current_level < max_depth_ - 1) {
                sTop++;
                stack[sTop] = dfuds_->get_num_children(current_node, current_node_pos, level_to_num_children[current_level]);

                current_node_pos += dfuds_->get_num_bits(current_node, current_level);
                current_level++;
            } 
            else {
                stack[sTop]--;

                if (current_level == max_depth_ - 1){

                    current_primary += dfuds_->get_num_children(current_node, current_node_pos, level_to_num_children[current_level]);
                }
                current_node_pos += dfuds_->get_num_bits(current_node, current_level);
            }
            current_node++;
            
            while (sTop >= 0 && stack[sTop] == 0) {
                sTop--;
                current_level--;
                if (sTop >= 0)
                    stack[sTop]--;
            }
        }
        node_pos = current_node_pos;
        return current_node;
    }

    // This function takes in a node (in preorder) and a symbol (branch index)
    // Return the child node (in preorder) designated by that symbol
    // This function differs from skip_children_subtree as it checks if that child node is present
    node_t child(tree_block *&p, node_t node, preorder_t &node_pos, symbol_t symbol, level_t &current_level,
                            preorder_t &current_frontier, preorder_t &current_primary) {

        if (node >= num_nodes_)
            return null_node;

        auto has_child = dfuds_->has_symbol(node, node_pos, symbol, level_to_num_children[current_level]);
        if (!has_child)
            return null_node;

        if (current_level == max_depth_ - 1)
            return node;

        node_t current_node;

        if (frontiers_ != nullptr && current_frontier < num_frontiers_ && node == get_preorder(current_frontier)) {

            p = get_pointer(current_frontier);
            current_frontier = 0;
            current_primary = 0;
            node_t temp_node = 0;
            preorder_t temp_node_pos = 0;

            current_node = p->skip_children_subtree(temp_node, temp_node_pos, symbol, current_level, current_frontier, current_primary);
            node_pos = temp_node_pos;

        } else {
            current_node = skip_children_subtree(node, node_pos, symbol, current_level, current_frontier, current_primary);
        }
        return current_node;
    }

    void insert(node_t node, node_t node_pos, data_point *leaf_point, level_t level,
                            preorder_t current_frontier, preorder_t current_primary, n_leaves_t primary_key) {
        
        symbol_t current_num_children = level_to_num_children[level];

        if (level == max_depth_) {
            
            current_num_children = level_to_num_children[level - 1];

            symbol_t parent_symbol = leaf_point->leaf_to_symbol(max_depth_ - 1);
            symbol_t tmp_symbol = dfuds_->next_symbol(0, node, node_pos, (1 << current_num_children) - 1, current_num_children);
            
            while (tmp_symbol != parent_symbol){    
                tmp_symbol = dfuds_->next_symbol(tmp_symbol + 1, node, node_pos, (1 << current_num_children) - 1, current_num_children);
                current_primary ++;
            }

            insert_primary_key_at_present_index(current_primary, primary_key);

            return;
        }

        node_t original_node = node;
        node_n_t original_node_pos = node_pos;
        node_n_t max_tree_nodes;

        if (root_depth_ <= max_depth_ / 2) max_tree_nodes = max_tree_nodes_ / 4;
        else if (root_depth_ <= max_depth_ / 4 * 3) max_tree_nodes = max_tree_nodes_ / 2;
        else max_tree_nodes = max_tree_nodes_;

        if (is_osm)
            max_tree_nodes = max_tree_nodes_;

        if (frontiers_ != nullptr && current_frontier < num_frontiers_ && node == get_preorder(current_frontier)) {
            
            preorder_t node_previous_bits = dfuds_->get_num_bits(node, level);
            if (dfuds_->get_num_children(node, node_pos, level_to_num_children[level]) >= 1){
                dfuds_->set_symbol(node, node_pos, leaf_point->leaf_to_symbol(level), false, level_to_num_children[level]);
            }
            else {
                dfuds_->set_symbol(node, node_pos, leaf_point->leaf_to_symbol(level), true, level_to_num_children[level]);
            }

            total_nodes_bits_ += dfuds_->get_num_bits(node, level) - node_previous_bits;
            get_pointer(current_frontier)->insert(0, 0, leaf_point, level, 0, 0, primary_key);

            return;
        }

        else if (level + 1 == max_depth_) {

            symbol_t next_symbol = leaf_point->leaf_to_symbol(level);

            preorder_t original_node_previous_bits = dfuds_->get_num_bits(original_node, level);            
            dfuds_->set_symbol(original_node, original_node_pos, next_symbol, false, level_to_num_children[level]);
            total_nodes_bits_ += dfuds_->get_num_bits(original_node, level) - original_node_previous_bits;

            symbol_t tmp_symbol = dfuds_->next_symbol(0, node, node_pos, (1 << current_num_children) - 1, current_num_children);
            
            while (tmp_symbol != next_symbol){

                tmp_symbol = dfuds_->next_symbol(tmp_symbol + 1, node, node_pos, (1 << current_num_children) - 1, current_num_children);
                current_primary ++;
            }

            insert_primary_key_at_index(current_primary, primary_key);

            return;
        }

        else if (num_nodes_ + (max_depth_ - level) - 1 <= node_capacity_) {
            
            symbol_t current_symbol = leaf_point->leaf_to_symbol(level);
            node = skip_children_subtree(node, node_pos, current_symbol, level, current_frontier, current_primary);

            preorder_t original_node_previous_bits = dfuds_->get_num_bits(original_node, level);
            dfuds_->set_symbol(original_node, original_node_pos, current_symbol, false, current_num_children);

            node_pos += dfuds_->get_num_bits(original_node, level) - original_node_previous_bits;
            total_nodes_bits_ += dfuds_->get_num_bits(original_node, level) - original_node_previous_bits;

            node_t from_node = num_nodes_ - 1;
            node_t from_node_pos = total_nodes_bits_;

            bool shifted = false;
            if (from_node >= node) {
            
                shifted = true;
                symbol_t total_bits_to_shift = 0;
                for (level_t i = level + 1; i < max_depth_; i++) {
                    total_bits_to_shift += level_to_num_children[i];    // Compressed Node Representation
                }

                dfuds_->shift_backward(node, node_pos, total_bits_to_shift, max_depth_ - level - 1);
                from_node = node;
                from_node_pos = node_pos;
            }
            else {
                from_node++;
            }

            level++;

            for (level_t current_level = level; current_level < max_depth_; current_level++) {

                if (!shifted)
                {
                    dfuds_->ClearWidth(from_node_pos, level_to_num_children[current_level], true);
                    dfuds_->ClearWidth(from_node, 1, false);
                }
                symbol_t next_symbol = leaf_point->leaf_to_symbol(current_level);
                dfuds_->set_symbol(from_node, from_node_pos, next_symbol, true, level_to_num_children[current_level]);                

                num_nodes_++;
                from_node_pos += dfuds_->get_num_bits(from_node, current_level);
                total_nodes_bits_ += dfuds_->get_num_bits(from_node, current_level);
                from_node++;
            }

            // shift the flags by length since all nodes have been shifted by that amount
            if (frontiers_ != nullptr)
                for (preorder_t j = current_frontier; j < num_frontiers_; j++){
                    set_preorder(j, get_preorder(j) + max_depth_ - level);
                    set_pointer(j, get_pointer(j));
                }

            insert_primary_key_at_index(current_primary, primary_key);
            return;
        } 
        else if (num_nodes_ + (max_depth_ - level) - 1 <= max_tree_nodes) 
        {
            // It is going to allocate more, but that's fine...
            uint32_t total_extra_bits = 0;
            for (unsigned int i = level; i < max_depth_; i++){
                total_extra_bits += level_to_num_children[i];
            }

            dfuds_->keep_bits(total_nodes_bits_, true);
            dfuds_->increase_bits(total_extra_bits, true);

            dfuds_->keep_bits(num_nodes_, false);
            dfuds_->increase_bits(max_depth_ - level, false);

            node_capacity_ = num_nodes_ + (max_depth_ - level);
            bit_capacity_ = total_nodes_bits_ + total_extra_bits;

            insert(node, node_pos, leaf_point, level, current_frontier, current_primary, primary_key);
            return;
        } 
        else {
            preorder_t subtree_size, selected_node_depth;
            preorder_t selected_node_pos = 0;
            preorder_t num_primary = 0, selected_primary_index = 0;
            preorder_t node_to_primary[4096] = {0};
            preorder_t node_to_depth[4096] = {0};

            node_t selected_node = select_subtree(subtree_size, selected_node_depth, selected_node_pos, num_primary, selected_primary_index, node_to_primary, node_to_depth);

            node_t orig_selected_node = selected_node;
            preorder_t orig_selected_node_pos = selected_node_pos;

            auto *new_dfuds = new compressed_bitmap::compressed_bitmap(subtree_size + 1, total_nodes_bits_);
            preorder_t frontier;
            //  Find the first frontier node > selected_node
            for (frontier = 0; frontier < num_frontiers_; frontier++)
                if (get_preorder(frontier) > selected_node)
                    break;

            preorder_t frontier_selected_node = frontier;
            node_t insertion_node = node;
            preorder_t insertion_node_pos = node_pos;

            node_t dest_node = 0;
            preorder_t dest_node_pos = 0;
            preorder_t n_nodes_copied = 0, copied_frontier = 0, copied_primary = 0;

            bool insertion_in_new_block = false;
            bool is_in_root = false;

            preorder_t new_pointer_index = 0;

            frontier_node *new_pointer_array = nullptr;
            if (num_frontiers_ > 0) {
                new_pointer_array = (frontier_node *) malloc(sizeof(frontier_node) * (num_frontiers_ + 5));
            }
            preorder_t current_frontier_new_block = 0;
            preorder_t current_primary_new_block = 0;
            preorder_t subtree_bits = 0;

            while (n_nodes_copied < subtree_size) {
                //  If we meet the current node (from which we want to do insertion)
                // insertion_node is the new preorder in new block where we want to insert a node
                if (selected_node == node) {
                    insertion_in_new_block = true;
                    if (dest_node != 0) {
                        insertion_node = dest_node;
                        insertion_node_pos = dest_node_pos;
                    }
                    else {
                        insertion_node = node;
                        insertion_node_pos = node_pos;
                        is_in_root = true;
                    }
                    current_frontier_new_block = copied_frontier;
                    current_primary_new_block = copied_primary;
                }
                // If we see a frontier node, copy pointer to the new block
                if (new_pointer_array != nullptr && frontier < num_frontiers_ && selected_node == get_preorder(frontier)) {

                    new_pointer_array[new_pointer_index].preorder_ = dest_node;
                    new_pointer_array[new_pointer_index].pointer_ = get_pointer(frontier);
                    
                    frontier++;
                    new_pointer_index++;
                    copied_frontier++;
                }

                if (node_to_primary[selected_node]){
                    copied_primary += node_to_primary[selected_node];
                }

                dfuds_->copy_node_cod(new_dfuds, selected_node, selected_node_pos, dest_node, dest_node_pos, level_to_num_children[node_to_depth[selected_node]]);
                subtree_bits += dfuds_->get_num_bits(selected_node, node_to_depth[selected_node]);
                dest_node_pos += dfuds_->get_num_bits(selected_node, node_to_depth[selected_node]);  // Still selected_node
                selected_node_pos += dfuds_->get_num_bits(selected_node, node_to_depth[selected_node]);
                selected_node += 1;
                dest_node += 1;
                n_nodes_copied += 1;
            }
            auto new_block = new tree_block(selected_node_depth, subtree_size, dest_node_pos, subtree_size, max_depth_, max_tree_nodes_, NULL);
            new_block->dfuds_ = new_dfuds;

            //  If no pointer is copied to the new block
            if (new_pointer_index == 0) {
                if (new_pointer_array != nullptr)
                    free(new_pointer_array);

                // Expand frontiers array to add one more frontier node
                frontiers_ = (frontier_node *) realloc(frontiers_, sizeof(frontier_node) * (num_frontiers_ + 1));

                // Shift right one spot to move the pointers from flagSelectedNode + 1 to nPtrs
                for (preorder_t j = num_frontiers_; j > frontier_selected_node; j--) {
                    set_preorder(j, get_preorder(j - 1) - subtree_size + 1);
                    set_pointer(j, get_pointer(j - 1));
                }
                //  Insert that new frontier node
                set_preorder(frontier_selected_node, orig_selected_node);
                set_pointer(frontier_selected_node, new_block);
                num_frontiers_++;

            } else {
                //  If there are pointers copied to the new block
                new_pointer_array = (frontier_node *) realloc(new_pointer_array,
                                                            sizeof(frontier_node) * (new_pointer_index));

                new_block->frontiers_ = new_pointer_array;
                new_block->num_frontiers_ = new_pointer_index;
                // Update pointer block parent pointer
                for (preorder_t j = 0; j < new_pointer_index; j++){
                    new_block->set_preorder(j, new_block->get_preorder(j));
                    new_block->set_pointer(j, new_block->get_pointer(j));
                }
                set_preorder(frontier_selected_node, orig_selected_node);
                set_pointer(frontier_selected_node, new_block);

                for (preorder_t j = frontier_selected_node + 1; frontier < num_frontiers_; j++, frontier++) {
                    set_preorder(j, get_preorder(frontier) - subtree_size + 1);
                    set_pointer(j, get_pointer(frontier));
                }
                num_frontiers_ = num_frontiers_ - copied_frontier + 1;
                frontiers_ = (frontier_node *) realloc(frontiers_, sizeof(frontier_node) * (num_frontiers_));
            }

            // Copy primary key to the new block
            for (preorder_t i = selected_primary_index; i < selected_primary_index + num_primary; i++){
                               
                new_block->primary_key_list.push_back(primary_key_list[i]);
                uint16_t primary_key_size = primary_key_list[i].size();

                for (uint16_t j = 0; j < primary_key_size; j++){
                    p_key_to_treeblock_compact.Set(primary_key_list[i].get(j), new_block);
                }
            }

            // Erase copied primary keys            
            primary_key_list.erase(std::next(primary_key_list.begin(), selected_primary_index), std::next(primary_key_list.begin(), selected_primary_index + num_primary));

            // Now, delete the subtree copied to the new block
            orig_selected_node_pos += dfuds_->get_num_bits(orig_selected_node, node_to_depth[orig_selected_node]);
            orig_selected_node++;

            if (selected_node < num_nodes_) {

                if (selected_node <= node) {
                    insertion_node = node - selected_node + orig_selected_node;
                    insertion_node_pos = node_pos - selected_node_pos + orig_selected_node_pos;
                }
                total_nodes_bits_ = total_nodes_bits_ - selected_node_pos + orig_selected_node_pos;
                dfuds_->shift_forward(selected_node, selected_node_pos, orig_selected_node, orig_selected_node_pos);
            }
            else if (selected_node >= num_nodes_){

                dfuds_->bulk_clear_node(orig_selected_node, orig_selected_node_pos, selected_node, selected_node_pos);
                total_nodes_bits_ -= selected_node_pos - orig_selected_node_pos;
            }
            if (node_capacity_ * level_to_num_children[root_depth_] < bit_capacity_){

                bit_capacity_ = node_capacity_ * level_to_num_children[root_depth_];
            }

            if (subtree_size > max_depth_) {
                node_capacity_ -= subtree_size - max_depth_; 
            } else {
                node_capacity_ -= subtree_size - 1;
            }

            dfuds_->keep_bits(node_capacity_, false);
            num_nodes_ -= (subtree_size - 1);

            if (insertion_node > orig_selected_node - 1 /*it was ++*/ && !insertion_in_new_block){
                current_frontier -= copied_frontier - 1;
            }
            // Update current primary
            if (current_primary >= selected_primary_index + num_primary){
                current_primary -= num_primary;
            }
            else if (current_primary >= selected_primary_index){
                current_primary = selected_primary_index;
            }

            // If the insertion continues in the new block
            if (insertion_in_new_block) {
                if (is_in_root) {
                    
                    preorder_t insertion_node_previous_bits = dfuds_->get_num_bits(insertion_node, level);
                    dfuds_->set_symbol(insertion_node, insertion_node_pos, leaf_point->leaf_to_symbol(level), false, dfuds_->get_num_bits(insertion_node, level));
                    total_nodes_bits_ += dfuds_->get_num_bits(insertion_node, level) - insertion_node_previous_bits;
                    

                    new_block->insert(0, 0, leaf_point, level, current_frontier_new_block, current_primary_new_block, primary_key);

                } else {
                    new_block->insert(insertion_node, insertion_node_pos, leaf_point, level, current_frontier_new_block, current_primary_new_block, primary_key);
                }
            }
            // If the insertion is in the old block
            else {
                insert(insertion_node, insertion_node_pos, leaf_point, level, current_frontier, current_primary, primary_key); 
            }
            return;
        }
    }
    // Traverse the current TreeBlock, going into frontier nodes as needed
    // Until it cannot traverse further and calls insertion
    void insert_remaining(data_point *leaf_point, level_t level, n_leaves_t primary_key) {
        
        node_t current_node = 0;
        preorder_t current_node_pos = 0;
        preorder_t current_frontier = 0;
        preorder_t current_primary = 0;

        node_t temp_node = 0;
        preorder_t temp_node_pos = 0;
        previous_p_key_ = primary_key;
        while (level < max_depth_) {
            tree_block *current_treeblock = this;

            temp_node = child(current_treeblock, current_node, temp_node_pos,
                                            leaf_point->leaf_to_symbol(level), level,
                                            current_frontier, current_primary);
            if (temp_node == (node_t) -1)
                break;

            current_node = temp_node;
            current_node_pos = temp_node_pos;
            if (current_node == num_nodes_){
                break;
            }

            if (num_frontiers() > 0 && current_frontier < num_frontiers() &&
                current_node == get_preorder(current_frontier)) {
                
                tree_block *next_block = get_pointer(current_frontier);
                next_block->insert_remaining(leaf_point, level + 1, primary_key);
                
                return;
            }
            level++;
        }
        insert(current_node, current_node_pos, leaf_point, level, current_frontier, current_primary, primary_key);
        current_leaves_inserted ++;
        return;
    }

    // This function is used for testing.
    // It differs from above as it only returns True or False.
    bool walk_tree_block(data_point *leaf_point, level_t level) {
        
        preorder_t current_frontier = 0;
        preorder_t current_primary = 0;
        node_t current_node = 0;
        node_t temp_node = 0;
        preorder_t temp_node_pos = 0;

        while (level < max_depth_) {
            symbol_t current_symbol = leaf_point->leaf_to_symbol(level);

            tree_block *current_treeblock = this;
            temp_node = child(current_treeblock, current_node, temp_node_pos, current_symbol, level,
                                            current_frontier, current_primary);

            if (temp_node == (node_t) -1){
                return false;
            }
            current_node = temp_node;
    
            if (num_frontiers() > 0 && current_frontier < num_frontiers() &&
                current_node == get_preorder(current_frontier)) {
                tree_block *next_block = get_pointer(current_frontier);

                return next_block->walk_tree_block(leaf_point, level + 1);
            }
            level++;
        }
        return true;
    }

    void get_node_path(node_t node, symbol_t *node_path) {

        if (node == 0){
            node_path[root_depth_] = dfuds_->next_symbol(0, 0, 0, (1 << level_to_num_children[root_depth_]) - 1, level_to_num_children[root_depth_]);
            if (parent_tree_block_){
                parent_tree_block_->get_node_path(treeblock_frontier_num_, node_path);
            }
            else {
                parent_trie_node_->get_node_path(root_depth_, node_path);
            }  
            return;          
        }

        preorder_t stack[35] = {};
        node_t path[35] = {};
        int symbol[35];
        level_t sTop_to_level[35] = {};

        for (uint8_t i = 0; i < 35; i++){
            symbol[i] = -1;
        }
        size_t node_positions[2048];
        node_positions[0] = 0;
        preorder_t current_frontier = 0;
        int sTop = 0;

        node_t top_node = 0;
        preorder_t top_node_pos = 0;

        symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, top_node, top_node_pos, (1 << level_to_num_children[root_depth_]) - 1, level_to_num_children[root_depth_]);

        stack[sTop] = dfuds_->get_num_children(0, 0, level_to_num_children[root_depth_]);
        sTop_to_level[sTop] = root_depth_;
        
        level_t current_level = root_depth_ + 1;
        node_t current_node = 1;
        preorder_t current_node_pos = dfuds_->get_num_bits(0, root_depth_);        

        if (frontiers_ != nullptr && current_frontier < num_frontiers_ && current_node > get_preorder(current_frontier))
            ++current_frontier;
        preorder_t next_frontier_preorder;

        if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
            next_frontier_preorder = -1;
        else
            next_frontier_preorder = get_preorder(current_frontier);

        while (current_node < num_nodes_ && sTop >= 0) {

            node_positions[current_node] = current_node_pos;
            current_node_pos += dfuds_->get_num_bits(current_node, current_level);

            if (current_node == next_frontier_preorder) {
                if (current_node != node){
                    top_node = path[sTop];
                    symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, top_node, node_positions[top_node], (1 << level_to_num_children[sTop_to_level[sTop]]) - 1,  level_to_num_children[sTop_to_level[sTop]]);
                }
                ++current_frontier;
                if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
                    next_frontier_preorder = -1;
                else
                    next_frontier_preorder = get_preorder(current_frontier);

                --stack[sTop];
            }
            // It is "-1" because current_level is 0th indexed.
            else if (current_level < max_depth_ - 1) 
            {
                sTop++;
                stack[sTop] = dfuds_->get_num_children(current_node, node_positions[current_node], level_to_num_children[current_level]);
                path[sTop] = current_node;
                sTop_to_level[sTop] = current_level;

                symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, current_node, node_positions[current_node], (1 << level_to_num_children[sTop_to_level[sTop]]) - 1, level_to_num_children[sTop_to_level[sTop]]);
                ++current_level;
            }
            else if (current_level == max_depth_ - 1 && stack[sTop] > 1 && current_node < node)
            {
                top_node = path[sTop];
                symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, top_node, node_positions[top_node], (1 << level_to_num_children[sTop_to_level[sTop]]) - 1, level_to_num_children[sTop_to_level[sTop]]);
                --stack[sTop];   
            } 
            else
            {
                --stack[sTop];
            }

            if (current_node == node){
                break;
            }

            ++current_node;
            bool backtracekd = false;
            while (sTop >= 0 && stack[sTop] == 0) {
                backtracekd = true;
                path[sTop] = 0;
                symbol[sTop] = -1;
                --sTop;
                --current_level;
                if (sTop >= 0)
                    --stack[sTop];
            }
            if (backtracekd){
                top_node = path[sTop];
                symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, top_node, node_positions[top_node], (1 << level_to_num_children[sTop_to_level[sTop]]) - 1, level_to_num_children[sTop_to_level[sTop]]);
            }
        }
        if (current_node == num_nodes_){
            fprintf(stderr, "node not found!\n");
            return;
        }
        for (int i = 0; i <= sTop; i++){
            node_path[root_depth_ + i] = symbol[i];
        }
        if (parent_tree_block_){
            parent_tree_block_->get_node_path(treeblock_frontier_num_, node_path);
        }
        else {
            parent_trie_node_->get_node_path(root_depth_, node_path);
        }
    }

    symbol_t get_node_path_primary_key(n_leaves_t primary_key, symbol_t *node_path) {

        preorder_t stack[35] = {};
        node_t path[35] = {};
        int symbol[35];
        level_t sTop_to_level[35] = {};

        for (uint8_t i = 0; i < 35; i++){
            symbol[i] = -1;
        }
        size_t node_positions[2048];  
        node_positions[0] = 0;
        int sTop = 0;
        node_t top_node = 0;
        symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, 0, 0, (1 << level_to_num_children[root_depth_]) - 1, level_to_num_children[root_depth_]);
        stack[sTop] = dfuds_->get_num_children(0, 0, level_to_num_children[root_depth_]);
        sTop_to_level[sTop] = root_depth_;

        level_t current_level = root_depth_ + 1;
        node_t current_node = 1;
        preorder_t current_node_pos = dfuds_->get_num_bits(0, root_depth_);

        preorder_t current_frontier = 0;
        preorder_t current_primary = 0;

        if (frontiers_ != nullptr && current_frontier < num_frontiers_ && current_node > get_preorder(current_frontier))
            ++current_frontier;
        preorder_t next_frontier_preorder;
        symbol_t parent_symbol = -1;
        if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
            next_frontier_preorder = -1;
        else
            next_frontier_preorder = get_preorder(current_frontier);

        while (current_node < num_nodes_ && sTop >= 0) {
            
            node_positions[current_node] = current_node_pos;
            current_node_pos += dfuds_->get_num_bits(current_node, current_level);

            if (current_node == next_frontier_preorder) {
                top_node = path[sTop];
                symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, top_node, node_positions[top_node], (1 << level_to_num_children[sTop_to_level[sTop]]) - 1, level_to_num_children[sTop_to_level[sTop]]);               
                ++current_frontier;
                if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
                    next_frontier_preorder = -1;
                else
                    next_frontier_preorder = get_preorder(current_frontier);

                --stack[sTop];
            }
            // It is "-1" because current_level is 0th indexed.
            else if (current_level < max_depth_ - 1) 
            {
                sTop++;
                stack[sTop] = dfuds_->get_num_children(current_node, node_positions[current_node], level_to_num_children[current_level]);
                path[sTop] = current_node;
                sTop_to_level[sTop] = current_level;

                symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, current_node, node_positions[current_node], (1 << level_to_num_children[sTop_to_level[sTop]]) - 1, level_to_num_children[sTop_to_level[sTop]]);
                ++current_level;
            }
            else
            {
                --stack[sTop];
                if (current_level == max_depth_ - 1){
                    
                    preorder_t new_current_primary = current_primary + dfuds_->get_num_children(current_node, node_positions[current_node], level_to_num_children[current_level]);
                    bool found = false;
                    for (preorder_t p = current_primary; p < new_current_primary; p ++)
                    {
                        if (primary_key_list[p].check_if_present(primary_key))
                        {
                            found = true;
                            // This optimization doesn't seem to be faster
                            parent_symbol = dfuds_->get_k_th_set_bit(current_node, p - current_primary /* 0-indexed*/, node_positions[current_node], level_to_num_children[current_level]);
                            break;                     
                        }
                    }
                    current_primary = new_current_primary;
                    if (!found && stack[sTop] > 0){
                        top_node = path[sTop];

                        symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, top_node, node_positions[top_node], (1 << level_to_num_children[sTop_to_level[sTop]]) - 1, level_to_num_children[sTop_to_level[sTop]]);                  

                    }
                    if (found){
                        break;
                    }                    
                }
            }
            ++current_node;
        
            bool backtraceked = false;
            while (sTop >= 0 && stack[sTop] == 0) {
                backtraceked = true;
                path[sTop] = 0;
                symbol[sTop] = -1;
                --sTop;
                --current_level;
                if (sTop >= 0)
                    --stack[sTop];
            }
            if (backtraceked){
                top_node = path[sTop];
                symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, top_node, node_positions[top_node], (1 << level_to_num_children[sTop_to_level[sTop]]) - 1, level_to_num_children[sTop_to_level[sTop]]);
            }
        }
        // This shouldn't happen
        if (current_node == num_nodes_){
            fprintf(stderr, "node not found!\n");
            return 0;
        }
        for (int i = 0; i <= sTop; i++){
            node_path[root_depth_ + i] = symbol[i];
        }
        if (parent_tree_block_){
            parent_tree_block_->get_node_path(treeblock_frontier_num_, node_path);
        }
        else {
            parent_trie_node_->get_node_path(root_depth_, node_path);
        }        
        return parent_symbol;
    }

    data_point *node_path_to_coordinates(symbol_t *node_path, dimension_t dimension) const{

        auto coordinates = new data_point();

        for (level_t i = 0; i < max_depth_; i++){
            symbol_t current_symbol = node_path[i];
            dimension_t current_symbol_pos = level_to_num_children[i] - 1;

            for (dimension_t j = 0; j < dimension; j++){

                if (dimension_to_num_bits[j] <= i)
                    continue;         

                level_t current_bit = GETBIT(current_symbol, current_symbol_pos);
                current_symbol_pos --;

                point_t coordinate = coordinates->get_coordinate(j);
                coordinate = (coordinate << 1) + current_bit;
                coordinates->set_coordinate(j, coordinate);
            }
        }
        return coordinates;
    }

    void range_search_treeblock(data_point *start_range, data_point *end_range, tree_block *current_block, level_t level, 
                                            preorder_t current_node, preorder_t current_node_pos,
                                            preorder_t prev_node, preorder_t prev_node_pos,
                                            node_t current_frontier, preorder_t current_primary, point_array *found_points) {

        if (level == max_depth_) {
            
            symbol_t parent_symbol = start_range->leaf_to_symbol(max_depth_ - 1);
            symbol_t tmp_symbol = dfuds_->next_symbol(0, prev_node, prev_node_pos, (1 << level_to_num_children[level - 1]) - 1, level_to_num_children[level - 1]);

            while (tmp_symbol != parent_symbol){    
                tmp_symbol = dfuds_->next_symbol(tmp_symbol + 1, prev_node, prev_node_pos, (1 << level_to_num_children[level - 1]) - 1, level_to_num_children[level - 1]);
                current_primary ++;
            }

            n_leaves_t list_size = primary_key_list[current_primary].size();
            for (n_leaves_t i = 0; i < list_size; i++)
            {
                auto primary_key = primary_key_list[current_primary].get(i);
                auto *leaf = new data_point();

                leaf->set(start_range->get());
                leaf->set_primary(primary_key);

                found_points->add_leaf(leaf);
            }
            return;
        }
                                        
        if (current_node >= num_nodes_){
            return;
        }

        if (num_frontiers() > 0 && current_frontier < num_frontiers() &&
            current_node == get_preorder(current_frontier)) {
            
            tree_block *new_current_block = get_pointer(current_frontier);
            node_t new_current_frontier = 0;
            preorder_t new_current_primary = 0;
            new_current_block->range_search_treeblock(start_range, end_range, new_current_block, level, 0, 0,0,0, new_current_frontier, new_current_primary, found_points); 
            return;
        }
        
        symbol_t start_range_symbol = start_range->leaf_to_symbol(level);
        symbol_t end_range_symbol = end_range->leaf_to_symbol(level);
        representation_t representation = start_range_symbol ^ end_range_symbol;
        representation_t neg_representation = ~representation;

        struct data_point original_start_range = (*start_range);
        struct data_point original_end_range = (*end_range); 

        preorder_t new_current_node;
        preorder_t new_current_node_pos = 0;
        tree_block *new_current_block;
        preorder_t new_current_frontier;
        preorder_t new_current_primary;

        symbol_t current_symbol = dfuds_->next_symbol(start_range_symbol, current_node, current_node_pos, end_range_symbol, level_to_num_children[level]);       

        while (current_symbol <= end_range_symbol){
            
            if (!dfuds_->has_symbol(current_node, current_node_pos, current_symbol, level_to_num_children[level])){
                continue;
            }

            if ((start_range_symbol & neg_representation) == (current_symbol & neg_representation)){
                
                new_current_block = current_block;
                new_current_frontier = current_frontier;
                new_current_primary = current_primary;
                new_current_node_pos = current_node_pos;

                new_current_node = current_block->child(new_current_block, current_node, new_current_node_pos, current_symbol, level,
                                                        new_current_frontier, new_current_primary);
                
                 start_range->update_symbol(end_range, current_symbol, level); // NOT HERE

                if (new_current_block != current_block){

                    new_current_block->range_search_treeblock(start_range, end_range, new_current_block, level + 1, new_current_node, new_current_node_pos, 0, 0, new_current_frontier, new_current_primary, found_points); // NOT HERE
                }
                else {

                    current_block->range_search_treeblock(start_range, end_range, current_block, level + 1, new_current_node, new_current_node_pos, current_node, current_node_pos, new_current_frontier, new_current_primary, found_points);     // NOT HERE      
                }
                (*start_range) = original_start_range;
                (*end_range) = original_end_range;    
            }
            current_symbol = dfuds_->next_symbol(current_symbol + 1, current_node, current_node_pos, end_range_symbol, level_to_num_children[level]);
        }

    }
    
    void insert_primary_key_at_present_index(n_leaves_t index, n_leaves_t primary_key){

        p_key_to_treeblock_compact.Set(primary_key, this);

        primary_key_list[index].push(primary_key);
      
    }

    void insert_primary_key_at_index(n_leaves_t index, n_leaves_t primary_key){

        p_key_to_treeblock_compact.Set(primary_key, this);

        auto primary_key_ptr = bits::compact_ptr(primary_key);
        primary_key_list.insert(primary_key_list.begin() + index, primary_key_ptr);

    }

    size_t Serialize(std::ostream& out) {
        size_t out_size = 0;

        // node_n_t max_tree_nodes_;
        out.write(reinterpret_cast<const char *>(&max_tree_nodes_), sizeof(node_n_t));
        out_size += sizeof(node_n_t);

        // level_t root_depth_;
        out.write(reinterpret_cast<const char *>(&root_depth_), sizeof(level_t));
        out_size += sizeof(level_t);

        // node_n_t num_nodes_;
        out.write(reinterpret_cast<const char *>(&num_nodes_), sizeof(node_n_t));
        out_size += sizeof(node_n_t);        

        // preorder_t total_nodes_bits_;
        out.write(reinterpret_cast<const char *>(&total_nodes_bits_), sizeof(preorder_t));
        out_size += sizeof(preorder_t);         

        // node_n_t node_capacity_;
        out.write(reinterpret_cast<const char *>(&node_capacity_), sizeof(node_n_t));
        out_size += sizeof(node_n_t);     

        // node_n_t bit_capacity_;
        out.write(reinterpret_cast<const char *>(&bit_capacity_), sizeof(node_n_t));
        out_size += sizeof(node_n_t);          

        // level_t max_depth_;
        out.write(reinterpret_cast<const char *>(&max_depth_), sizeof(level_t));
        out_size += sizeof(level_t);

        // compressed_bitmap::compressed_bitmap *dfuds_{};
        uint64_t before = out_size;
        out_size += dfuds_->Serialize(out); 
        treeblock_nodes_serialized_size += out_size - before;

        // node_n_t num_frontiers_;
        out.write(reinterpret_cast<const char *>(&num_frontiers_), sizeof(node_n_t));
        out_size += sizeof(node_n_t);   

        // frontier_node *frontiers_ = nullptr; 
        out.write(reinterpret_cast<const char *>(frontiers_),
                    sizeof(frontier_node) * num_frontiers_);        
        out_size += (num_frontiers_ * sizeof(frontier_node)); 

        // preorder_t previous_p_key_;
        out.write(reinterpret_cast<const char *>(&previous_p_key_), sizeof(preorder_t));
        out_size += sizeof(preorder_t);          

        //  tree_block *parent_tree_block_
        out.write(reinterpret_cast<const char *>(&parent_tree_block_), sizeof(tree_block *));
        out_size += sizeof(tree_block *);          

        //  trie_node *parent_trie_node_
        out.write(reinterpret_cast<const char *>(&parent_trie_node_), sizeof(trie_node *));
        out_size += sizeof(trie_node *);     

        // preorder_t treeblock_frontier_num_ = 0;
        out.write(reinterpret_cast<const char *>(&treeblock_frontier_num_), sizeof(preorder_t));
        out_size += sizeof(preorder_t);    

        uint64_t out_prev = out_size;
        uint32_t primary_key_list_size = primary_key_list.size();
        out.write(reinterpret_cast<const char*>(&primary_key_list_size), sizeof(primary_key_list_size));
        out_size += sizeof(primary_key_list_size); 

        out.write(reinterpret_cast<const char*>(primary_key_list.data()), primary_key_list_size * sizeof(bits::compact_ptr));
        out_size += primary_key_list_size * sizeof(bits::compact_ptr);
        primary_key_ptr_vector_serialized_size += out_size - out_prev;
        out_prev = out_size;

        for (uint16_t i = 0; i < primary_key_list_size; i++){
            out_size += primary_key_list[i].Serialize(out);
        }
        primary_key_list_serialized_size += out_size - out_prev;

        for (uint16_t i = 0; i < num_frontiers_; i++){
            out_size += ((frontier_node *) frontiers_)[i].pointer_->Serialize(out);
        }
        return out_size;
    }

    size_t Deserialize(std::istream &in) {
        size_t in_size  = 0;

        // node_n_t max_tree_nodes_;
        in.read(reinterpret_cast<char *>(&max_tree_nodes_), sizeof(node_n_t));
        in_size += sizeof(node_n_t);

        // level_t root_depth_;
        in.read(reinterpret_cast<char *>(&root_depth_), sizeof(level_t));
        in_size += sizeof(level_t);

        // node_n_t num_nodes_;
        in.read(reinterpret_cast<char *>(&num_nodes_), sizeof(node_n_t));
        in_size += sizeof(node_n_t);        

        // preorder_t total_nodes_bits_;
        in.read(reinterpret_cast<char *>(&total_nodes_bits_), sizeof(preorder_t));
        in_size += sizeof(preorder_t);         

        // node_n_t node_capacity_;
        in.read(reinterpret_cast<char *>(&node_capacity_), sizeof(node_n_t));
        in_size += sizeof(node_n_t);     

        // node_n_t bit_capacity_;
        in.read(reinterpret_cast<char *>(&bit_capacity_), sizeof(node_n_t));
        in_size += sizeof(node_n_t);          

        // level_t max_depth_;
        in.read(reinterpret_cast<char *>(&max_depth_), sizeof(level_t));
        in_size += sizeof(level_t);

        // compressed_bitmap::compressed_bitmap *dfuds_{};
        in_size += dfuds_->Deserialize(in); 

        // node_n_t num_frontiers_;
        in.read(reinterpret_cast<char *>(&num_frontiers_), sizeof(node_n_t));
        in_size += sizeof(node_n_t);    

        // frontier_node *frontiers_ = nullptr; 
        frontiers_ = static_cast<frontier_node *>(malloc(num_frontiers_ * sizeof(frontier_node)));
        in.read(reinterpret_cast<char *>(frontiers_),
                    sizeof(frontier_node) * num_frontiers_);        
        in_size += (num_frontiers_ * sizeof(frontier_node));

        // preorder_t previous_p_key_;
        in.read(reinterpret_cast<char *>(&previous_p_key_), sizeof(preorder_t));
        in_size += sizeof(preorder_t);          

        //  tree_block *parent_tree_block_
        in.read(reinterpret_cast<char *>(&parent_tree_block_), sizeof(tree_block *));
        in_size += sizeof(tree_block *);          

        //  trie_node *parent_trie_node_
        in.read(reinterpret_cast<char *>(&parent_trie_node_), sizeof(trie_node *));
        in_size += sizeof(trie_node *);     

        // preorder_t treeblock_frontier_num_ = 0;
        in.read(reinterpret_cast<char *>(&treeblock_frontier_num_), sizeof(preorder_t));
        in_size += sizeof(preorder_t);    

        // std::vector<bits::compact_ptr> primary_key_list;
        uint32_t primary_key_list_size;
        in.read(reinterpret_cast<char *>(&primary_key_list_size), sizeof(primary_key_list_size));
        in_size += sizeof(primary_key_list_size); // sizeof(primary_key_list_size) => 8

        primary_key_list.resize(primary_key_list_size);
        in.read(reinterpret_cast<char *>(primary_key_list.data()), primary_key_list_size * sizeof(bits::compact_ptr));
        in_size += primary_key_list_size * sizeof(bits::compact_ptr);

        for (uint16_t i = 0; i < primary_key_list_size; i++){
            in_size += primary_key_list[i].Deserialize(in);
        }

        for (uint16_t i = 0; i < num_frontiers_; i++){
            in_size += ((frontier_node *) frontiers_)[i].pointer_->Deserialize(in);
        }
        return in_size;
    }

    uint64_t size() {
        
        for (uint16_t i = 0; i < num_nodes_; i ++){

            if (dfuds_->is_collapse(i))
                collapsed_node_num ++;
        }


        total_treeblock_num ++; // For primary key -> treeblock index -> treeblock pointer
        
        uint64_t total_size = 0;
        total_size += sizeof(uint8_t); // root_depth_
        total_size += sizeof(uint16_t); // node_capacity_
        total_size += sizeof(uint64_t); // Either a treeblock pointer or trie node pointer + preorder number

        treeblock_primary_pointer_size += sizeof(primary_key_list) + primary_key_list.size() * 46 / 8;
        total_size += sizeof(primary_key_list) + primary_key_list.size() * 46 / 8 /*sizeof(bits::compact_ptr)*/;

        for (preorder_t i = 0; i < primary_key_list.size(); i++)
        {
            treeblock_primary_size +=primary_key_list[i].size_overhead();
            total_size += primary_key_list[i].size_overhead();
        }

        treeblock_nodes_size += dfuds_->size();
        total_size += dfuds_->size() /*+ sizeof(dfuds_)*/;
        total_size += num_frontiers_ * sizeof(tree_block *) /*Use compact pointer representation*/ + sizeof(frontiers_) /*pointer*/;

        for (uint16_t i = 0; i < num_frontiers_; i++){
            total_size += ((frontier_node *) frontiers_)[i].pointer_->size();
        }
        return total_size;
    }

private:

    node_n_t max_tree_nodes_;
    level_t root_depth_;
    node_n_t num_nodes_;
    preorder_t total_nodes_bits_;
    node_n_t node_capacity_;
    node_n_t bit_capacity_;
    level_t max_depth_;
    compressed_bitmap::compressed_bitmap *dfuds_{};
    frontier_node *frontiers_ = nullptr; 
    node_n_t num_frontiers_ = 0;
    preorder_t previous_p_key_ = 0;

    // TODO: parent_tree_block & parent_trie_node pointer can be combined
    tree_block *parent_tree_block_ = NULL;
    trie_node *parent_trie_node_ = NULL;

    preorder_t treeblock_frontier_num_ = 0;
    std::vector<bits::compact_ptr> primary_key_list;
};

#endif //MD_TRIE_TREE_BLOCK_H
