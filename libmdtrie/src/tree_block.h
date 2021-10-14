#ifndef MD_TRIE_TREE_BLOCK_H
#define MD_TRIE_TREE_BLOCK_H

#include "point_array.h"
#include "trie_node.h"
#include "compressed_bitmap.h"
#include <sys/time.h>
#include <cmath>
#include <mutex>
#include <shared_mutex>
#include "delta_encoded_array.h"
#include "elias_gamma_encoder.h"
#include "compact_ptr.h"

uint64_t get_bit_count = 0;
// uint64_t v2_storage_save_pos = 0;
// uint64_t v2_storage_save_neg = 0;
// uint64_t single_node_count = 0;
// uint64_t total_number_nodes = 0;

template<dimension_t DIMENSION, symbol_t NUM_BRANCHES>
class tree_block {
public:
    
    explicit tree_block(level_t root_depth, node_n_t tree_capacity, node_n_t num_nodes,
                        level_t max_depth, node_n_t max_tree_nodes, trie_node<DIMENSION, NUM_BRANCHES> *parent_trie_node = NULL) {
        root_depth_ = root_depth;
        tree_capacity_ = tree_capacity;

        max_depth_ = max_depth;
        max_tree_nodes_ = max_tree_nodes;
        num_nodes_ = num_nodes;
        dfuds_ = new compressed_bitmap::compressed_bitmap(tree_capacity_ + 1, DIMENSION);

        if (parent_trie_node){
            parent_trie_node_ = parent_trie_node;
        }
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

    bool check_frontiers(){

        for (preorder_t i = 1; i < num_frontiers_; i++){
            if (frontiers_[i].preorder_ <= frontiers_[i-1].preorder_)
                return false;
        }
        return true;
    }

    // This function selects the subTree starting from node 0
    // The selected subtree has the maximum subtree size
    node_t select_subtree(preorder_t &subtree_size, preorder_t &selected_node_depth, preorder_t &num_primary, preorder_t &selected_primary_index, preorder_t *index_to_primary) {

        // index -> Number of children & preorder
        node_info index_to_node[4096];

        // index -> size of subtree & preorder
        subtree_info index_to_subtree[4096];

        num_primary = 0;
        // preorder_t selected_primary_index_from_scratch = 0;

        // Index -> depth of the node
        preorder_t index_to_depth[4096];

        //  Corresponds to index_to_node, index_to_subtree, index_to_depth
        preorder_t node_stack_top = 0, subtree_stack_top = 0, depth_stack_top = 0;
        preorder_t depth;
        preorder_t current_frontier = 0;
        selected_primary_index = 0;

        index_to_node[node_stack_top].preorder_ = 0;
        index_to_node[node_stack_top++].n_children_ = dfuds_->get_n_children(0);
        depth = root_depth_ + 1;
        preorder_t next_frontier_preorder;

        if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
            next_frontier_preorder = -1;
        else
            next_frontier_preorder = get_preorder(current_frontier);

        for (preorder_t i = 1; i < num_nodes_; ++i) {
            // fprintf(stderr, "i: %ld, depth: %ld\n", i, depth);
            if (depth == max_depth_ - 1){
                index_to_primary[i] = dfuds_->get_n_children(i);
                // selected_primary_index_from_scratch += index_to_primary[i];
            }

            // If meet a frontier node
            if (i == next_frontier_preorder) {
                ++current_frontier;
                if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
                    next_frontier_preorder = -1;
                else
                    next_frontier_preorder = get_preorder(current_frontier);
                --index_to_node[node_stack_top - 1].n_children_;
            }
                //  Start searching for its children
            else if (depth < max_depth_ - 1) {
                index_to_node[node_stack_top].preorder_ = i;
                index_to_node[node_stack_top++].n_children_ = dfuds_->get_n_children(i);
                depth++;
            }
                //  Reached the maxDepth level
            else{
                // TODO: See if this is correct

                --index_to_node[node_stack_top - 1].n_children_;
            }
                
            while (node_stack_top > 0 && index_to_node[node_stack_top - 1].n_children_ == 0) {
                index_to_subtree[subtree_stack_top].preorder_ = index_to_node[node_stack_top - 1].preorder_;

                index_to_subtree[subtree_stack_top++].subtree_size_ = i - index_to_node[node_stack_top - 1].preorder_ + 1;
                --node_stack_top;
                index_to_depth[depth_stack_top++] = --depth;
                if (node_stack_top == 0)
                    break;
                else
                    index_to_node[node_stack_top - 1].n_children_--;
            }
        }
        // Now I have to go through the index_to_subtree vector to choose the proper subtree
        preorder_t min_node = 0;
        preorder_t min = (preorder_t) -1;
        preorder_t min_index = 0;
        preorder_t diff = (preorder_t) -1;

        auto leftmost = (preorder_t) -1;

        for (preorder_t i = 0; i < subtree_stack_top; ++i) {
            auto subtree_size_at_i = (preorder_t) index_to_subtree[i].subtree_size_;
            if (index_to_subtree[i].preorder_ != 0 && num_nodes_ <= subtree_size_at_i * 4 && subtree_size_at_i * 4 <= 3 * num_nodes_ &&
                index_to_subtree[i].preorder_ < leftmost) {

                // TODO: root node's subtree size doesn't match num_nodes_
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

            for (preorder_t i = 1; i < subtree_stack_top; ++i) {
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
            selected_primary_index += index_to_primary[i];
        }

        for (preorder_t i = min_node; i < min_node + subtree_size; i ++){
            num_primary += index_to_primary[i];
        }

        if (min_node == 0){
            raise(SIGINT);
        }

        return min_node;
    }

    // This function inserts the string at the node position
    void insert(node_t node, data_point<DIMENSION> *leaf_point, level_t level, level_t length,
                            preorder_t current_frontier, preorder_t current_primary, n_leaves_t primary_key) {

        // mutex.lock();

        if (level == length) {

            symbol_t parent_symbol = leaf_point->leaf_to_symbol(max_depth_ - 1, max_depth_);
            symbol_t tmp_symbol = dfuds_->next_symbol(0, node, NUM_BRANCHES - 1);
            
            while (tmp_symbol != parent_symbol){    
                tmp_symbol = dfuds_->next_symbol(tmp_symbol + 1, node, NUM_BRANCHES - 1);
                current_primary ++;
            }

            insert_primary_key_at_present_index(current_primary, primary_key);
            // mutex.unlock();

            return;
        }

        node_t original_node = node;
        node_n_t max_tree_nodes;

        if (root_depth_ <=/*=*/ 16) max_tree_nodes = 64;
        else if (root_depth_ <= 24) max_tree_nodes = 128;
        else max_tree_nodes = max_tree_nodes_;

        if (is_osm)
            max_tree_nodes = max_tree_nodes_;

        //  node is a frontier node
        if (frontiers_ != nullptr && current_frontier < num_frontiers_ && node == get_preorder(current_frontier)) {
            
            if (dfuds_->get_n_children(node) >= 1){

                dfuds_->set_symbol(node, leaf_point->leaf_to_symbol(level, max_depth_), false);
            }
            else {
                dfuds_->set_symbol(node, leaf_point->leaf_to_symbol(level, max_depth_), true);
            }
    
            // mutex.unlock();
            get_pointer(current_frontier)->mutex.lock();
            get_pointer(current_frontier)->insert(0, leaf_point, level, length, 0, 0, primary_key);
            get_pointer(current_frontier)->mutex.unlock();
            return;
        }
        //  If there is only one character left
        //  Insert that character into the correct position
        else if (length - level == 1) {

            symbol_t next_symbol = leaf_point->leaf_to_symbol(level, max_depth_);

            dfuds_->set_symbol(original_node, next_symbol, false);
            
            symbol_t tmp_symbol = dfuds_->next_symbol(0, node, NUM_BRANCHES - 1);
            
            while (tmp_symbol != next_symbol)
            {
                tmp_symbol = dfuds_->next_symbol(tmp_symbol + 1, node, NUM_BRANCHES - 1);
                current_primary ++;
            }

            insert_primary_key_at_index(current_primary, primary_key);
            // mutex.unlock();
            return;
        }
            // there is room in current block for new nodes
        else if (num_nodes_ + (length - level) - 1 <= tree_capacity_) {
            // skip_children_subtree returns the position under node where the new str[0] will be inserted
            symbol_t current_symbol = leaf_point->leaf_to_symbol(level, max_depth_);
            node = skip_children_subtree(node, current_symbol, level, current_frontier, current_primary);

            dfuds_->set_symbol(original_node, current_symbol, false);
            node_t from_node = num_nodes_ - 1;

            //  In this while loop, we are making space for str
            //  By shifting nodes to the right of str[i] by len(str) spots

            bool shifted = false;
            if (from_node >= node) {
                shifted = true;
                dfuds_->shift_backward(node, length - level - 1);
                from_node = node;
            }
            else {
                from_node++;
                // TODO: bulk clear node doesn't work
                // dfuds_->bulk_clear_node(from_node, from_node + length - level - 2);
            }
            level++;

            //  Insert all remaining characters (Remember length -- above)

            for (level_t i = level; i < length; i++) {
                if (!shifted)
                {
                    dfuds_->clear_node(from_node);
                }
                symbol_t next_symbol = leaf_point->leaf_to_symbol(i, max_depth_);
                dfuds_->set_symbol(from_node, next_symbol, true);                

                num_nodes_++;
                from_node++;
            }
            // shift the flags by length since all nodes have been shifted by that amount
            if (frontiers_ != nullptr)
                for (preorder_t j = current_frontier; j < num_frontiers_; ++j){
                    set_preorder(j, get_preorder(j) + length - level);
                    set_pointer(j, get_pointer(j));
                }

            insert_primary_key_at_index(current_primary, primary_key);
            
            if (!check_frontiers())
                raise(SIGINT);

            // mutex.unlock();
            return;

        } else if (num_nodes_ + (length - level) - 1 <= max_tree_nodes) 
        {
            dfuds_->realloc_bitmap(num_nodes_ + length - level);

            tree_capacity_ = num_nodes_ + (length - level);
            // mutex.unlock();
            insert(node, leaf_point, level, length, current_frontier, current_primary, primary_key);
            return;

        } else {

            preorder_t subtree_size, selected_node_depth;
            preorder_t num_primary = 0, selected_primary_index = 0;
            preorder_t index_to_primary[4096] = {0};

            node_t selected_node = select_subtree(subtree_size, selected_node_depth, num_primary, selected_primary_index, index_to_primary);

            node_t orig_selected_node = selected_node;
            auto *new_dfuds = new compressed_bitmap::compressed_bitmap(tree_capacity_ + 1, DIMENSION);

            preorder_t frontier;
            //  Find the first frontier node > selected_node
            for (frontier = 0; frontier < num_frontiers_; frontier++)
                if (get_preorder(frontier) > selected_node)
                    break;

            preorder_t frontier_selected_node = frontier;
            node_t insertion_node = node;

            node_t dest_node = 0;
            preorder_t n_nodes_copied = 0, copied_frontier = 0, copied_primary = 0;

            bool insertion_in_new_block = false;
            bool is_in_root = false;

            preorder_t new_pointer_index = 0;

            frontier_node<DIMENSION> *new_pointer_array = nullptr;
            if (num_frontiers_ > 0) {
                new_pointer_array = (frontier_node<DIMENSION> *) malloc(sizeof(frontier_node<DIMENSION>) * (num_frontiers_ + 5));
            }
            preorder_t current_frontier_new_block = 0;
            preorder_t current_primary_new_block = 0;
            while (n_nodes_copied < subtree_size) {
                //  If we meet the current node (from which we want to do insertion)
                // insertion_node is the new preorder in new block where we want to insert a node
                if (selected_node == node) {
                    insertion_in_new_block = true;
                    if (dest_node != 0)
                        insertion_node = dest_node;
                    else {
                        insertion_node = node;
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

                if (index_to_primary[selected_node]){
                    copied_primary += index_to_primary[selected_node];
                }

                dfuds_->copy_node_cod(new_dfuds, selected_node, dest_node);

                selected_node += 1;
                dest_node += 1;
                n_nodes_copied += 1;
            }
 
            auto new_block = new tree_block(selected_node_depth, subtree_size, subtree_size, max_depth_, max_tree_nodes_);
            new_block->mutex.lock();
            new_block->dfuds_ = new_dfuds;
            new_block->mutex.unlock();

            //  If no pointer is copied to the new block
            if (new_pointer_index == 0) {
                if (new_pointer_array != nullptr)
                    free(new_pointer_array);

                // Expand frontiers array to add one more frontier node
                frontiers_ = (frontier_node<DIMENSION> *) realloc(frontiers_, sizeof(frontier_node<DIMENSION>) * (num_frontiers_ + 1));

                // Shift right one spot to move the pointers from flagSelectedNode + 1 to nPtrs
                for (preorder_t j = num_frontiers_; j > frontier_selected_node; --j) {
                    set_preorder(j, get_preorder(j - 1) - subtree_size + 1);
                    set_pointer(j, get_pointer(j - 1));
                }
                //  Insert that new frontier node
                set_preorder(frontier_selected_node, orig_selected_node);
                set_pointer(frontier_selected_node, new_block);
                num_frontiers_++;

                if (!check_frontiers())
                    raise(SIGINT);

            } else {
                //  If there are pointers copied to the new block
                new_pointer_array = (frontier_node<DIMENSION> *) realloc(new_pointer_array,
                                                            sizeof(frontier_node<DIMENSION>) * (new_pointer_index));

                new_block->mutex.lock();
                new_block->frontiers_ = new_pointer_array;
                new_block->num_frontiers_ = new_pointer_index;
                // Update pointer block parent pointer
                for (preorder_t j = 0; j < new_pointer_index; j++){
                    new_block->set_preorder(j, new_block->get_preorder(j));
                    new_block->set_pointer(j, new_block->get_pointer(j));
                }
                new_block->mutex.unlock();

                set_preorder(frontier_selected_node, orig_selected_node);
                set_pointer(frontier_selected_node, new_block);

                for (preorder_t j = frontier_selected_node + 1; frontier < num_frontiers_; j++, frontier++) {
                    set_preorder(j, get_preorder(frontier) - subtree_size + 1);
                    set_pointer(j, get_pointer(frontier));
                }
                num_frontiers_ = num_frontiers_ - copied_frontier + 1;
                frontiers_ = (frontier_node<DIMENSION> *) realloc(frontiers_, sizeof(frontier_node<DIMENSION>) * (num_frontiers_));

                if (!check_frontiers())
                    raise(SIGINT);
            }

            // Copy primary key to the new block
            for (preorder_t i = selected_primary_index; i < selected_primary_index + num_primary; i++){
                
                // TimeStamp start = GetTimestamp();
                
                new_block->primary_key_list.push_back(primary_key_list[i]);

                uint16_t primary_key_size = primary_key_list[i].size();
                for (uint16_t j = 0; j < primary_key_size; j++){
                    p_key_to_treeblock_compact.Set(primary_key_list[i].get(j), new_block);
                }
            }
            // Erase copied primary keys            
            primary_key_list.erase(std::next(primary_key_list.begin(), selected_primary_index), std::next(primary_key_list.begin(), selected_primary_index + num_primary));

            // Now, delete the subtree copied to the new block
  
            orig_selected_node++;

            if (selected_node < num_nodes_) {
                // Fixed a bug here: was node < num_nodes_
                if (selected_node <= node /* && node <= num_nodes_ */) {
                    insertion_node = node - selected_node + orig_selected_node;
                }
                dfuds_->shift_forward(selected_node, orig_selected_node);
            }

            else if(selected_node >= num_nodes_){
                dfuds_->bulk_clear_node(orig_selected_node, selected_node - 1);
            }

            if (subtree_size > length) {
                dfuds_->realloc_bitmap(tree_capacity_ - (subtree_size - length));
                tree_capacity_ -= subtree_size - length;

            } else {
                dfuds_->realloc_bitmap(tree_capacity_ - (subtree_size - 1));
                tree_capacity_ -= subtree_size - 1;
            }
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
 
                    dfuds_->set_symbol(insertion_node, leaf_point->leaf_to_symbol(level, max_depth_), false);
                    new_block->mutex.lock();
                    new_block->insert(0, leaf_point, level, length, current_frontier_new_block, current_primary_new_block, primary_key);
                    new_block->mutex.unlock();

                } else {
                    new_block->mutex.lock();
                    new_block->insert(insertion_node, leaf_point, level, length, current_frontier_new_block, current_primary_new_block, primary_key);
                    new_block->mutex.unlock();
                }
            }
            // If the insertion is in the old block
            else {
                insert(insertion_node, leaf_point, level, length, current_frontier, current_primary, primary_key); 
            }
            return;
        }
    }

    // This function takes in a node (in preorder) and a symbol (branch index)
    // Return the child node (in preorder) designated by that symbol
    node_t skip_children_subtree(node_t node, symbol_t symbol, level_t current_level,
                                            preorder_t &current_frontier, preorder_t &current_primary)  {
        
        if (current_level == max_depth_){
            return node;
        }

        int sTop = -1;
        preorder_t n_children_skip = dfuds_->get_child_skip(node, symbol);
        preorder_t n_children = dfuds_->get_n_children(node);
        preorder_t diff = n_children - n_children_skip;
        preorder_t stack[100];
        stack[++sTop] = n_children;

        node_t current_node = node + 1;

        if (frontiers_ != nullptr && current_frontier < num_frontiers_ && current_node > get_preorder(current_frontier))
            ++current_frontier;
        preorder_t next_frontier_preorder;

        if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
            next_frontier_preorder = -1;
        else
            next_frontier_preorder = get_preorder(current_frontier);

        ++current_level;
        while (current_node < num_nodes_ && sTop >= 0 && diff < stack[0]) {
            if (current_node == next_frontier_preorder) {
                ++current_frontier;
                if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
                    next_frontier_preorder = -1;
                else
                    next_frontier_preorder = get_preorder(current_frontier);
                --stack[sTop];
            }
            // It is "-1" because current_level is 0th indexed.
            else if (current_level < max_depth_ - 1) {
                stack[++sTop] = dfuds_->get_n_children(current_node);
                ++current_level;
            } else {
                --stack[sTop];

                // TODO: GDB debug
                // If immediate parent to the node
                if (current_level == max_depth_ - 1){

                    current_primary += dfuds_->get_n_children(current_node);

                }
            }

            ++current_node;
            while (sTop >= 0 && stack[sTop] == 0) {
                --sTop;
                --current_level;
                if (sTop >= 0)
                    --stack[sTop];
            }
        }
        return current_node;
    }

    // This function takes in a node (in preorder) and a symbol (branch index)
    // Return the child node (in preorder) designated by that symbol
    // This function differs from skip_children_subtree as it checks if that child node is present
    node_t child(tree_block *&p, node_t node, symbol_t symbol, level_t &current_level,
                            preorder_t &current_frontier, preorder_t &current_primary) {

        if (node >= num_nodes_)
            return null_node;

        get_bit_count ++;
        auto has_child = dfuds_->has_symbol(node, symbol);
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
            p->mutex.lock();
            current_node = p->skip_children_subtree(temp_node, symbol, current_level, current_frontier, current_primary);
            p->mutex.unlock();
        } else
            current_node = skip_children_subtree(node, symbol, current_level, current_frontier, current_primary);

        return current_node;
    }

    // Traverse the current TreeBlock, going into frontier nodes as needed
    // Until it cannot traverse further and calls insertion
    void insert_remaining(data_point<DIMENSION> *leaf_point, level_t length, level_t level, n_leaves_t primary_key, node_n_t node_pos) {
       
        mutex.lock();
        node_t current_node = 0;
        preorder_t current_frontier = 0;
        preorder_t current_primary = 0;

        node_t temp_node = 0;

        while (level < length) {
            tree_block *current_treeblock = this;

            temp_node = child(current_treeblock, current_node, node_pos,
                                            leaf_point->leaf_to_symbol(level, max_depth_), level,
                                            current_frontier, current_primary);
            if (temp_node == (node_t) -1)
                break;

            current_node = temp_node;
            if (current_node == num_nodes_){
                break;
            }

            /**
                TODO: consider the scenario where we go through the frontier node
                But at node 0, symbol is not found;
                Have to update the symbol at the previous treeblock as well.
            */               
            if (num_frontiers() > 0 && current_frontier < num_frontiers() &&
                current_node == get_preorder(current_frontier)) {
                
                tree_block<DIMENSION, NUM_BRANCHES> *next_block = get_pointer(current_frontier);
                mutex.unlock();
                next_block->insert_remaining(leaf_point, length, level + 1, primary_key, 0);
                return;
            }
            level++;
        }
        insert(current_node, node_pos, leaf_point, level, length, current_frontier, current_primary, primary_key);
        current_leaves_inserted ++;

        mutex.unlock();
        return;
    }

    // This function is used for testing.
    // It differs from above as it only returns True or False.
    bool walk_tree_block(data_point<DIMENSION> *leaf_point, level_t length, level_t level) {
        preorder_t current_frontier = 0;
        preorder_t current_primary = 0;
        node_t current_node = 0;
        node_t temp_node = 0;

        mutex.lock();

        while (level < length) {
            symbol_t current_symbol = leaf_point->leaf_to_symbol(level, max_depth_);

            tree_block *current_treeblock = this;
            temp_node = child(current_treeblock, current_node, current_symbol, level,
                                            current_frontier, current_primary);
            
            if (temp_node == (node_t) -1){
                mutex.unlock();
                return false;
            }
            current_node = temp_node;
    
            if (num_frontiers() > 0 && current_frontier < num_frontiers() &&
                current_node == get_preorder(current_frontier)) {
                tree_block<DIMENSION, NUM_BRANCHES> *next_block = get_pointer(current_frontier);
                mutex.unlock();
                return next_block->walk_tree_block(leaf_point, length, level + 1);
            }
            level++;
        }
        mutex.unlock();
        return true;
    }


    uint64_t size() {
        
        uint64_t total_size = sizeof(uint8_t) * 1 /*root depth, max_depth can be hard coded*/+ sizeof(uint16_t) * 2 /*max tree nodes (can be hard coded), num nodes & tree capacity can be potentially merged, num frontiers*/;

        // Using compact representation, I just need to store one pointer
        total_size += sizeof(tree_block *) /*+ sizeof(trie_node<DIMENSION> *) and preorder number*/;

        total_size += sizeof(primary_key_list) + ((primary_key_list.size() * 46) / 64 + 1) * 8;

        for (preorder_t i = 0; i < primary_key_list.size(); i++)
        {
            vector_size += primary_key_list[i].size_overhead();
            total_size += primary_key_list[i].size_overhead();
        }

        for (preorder_t i = 0; i < num_nodes_; i++){
            
            preorder_t num_children = dfuds_->get_n_children(i);
            if (node_children_to_occurrences.find(num_children) == node_children_to_occurrences.end())
                node_children_to_occurrences[num_children] = 1;
            else
                node_children_to_occurrences[num_children] += 1;
        }

        treeblock_nodes_size += dfuds_->size(); // TODO: not store pointer at dfuds
        total_size += dfuds_->size() /*+ sizeof(dfuds_)*/;

        total_size += num_frontiers_ * sizeof(tree_block *) /*Use compact pointer representation*/ + sizeof(frontiers_) /*pointer*/;

        for (uint16_t i = 0; i < num_frontiers_; i++)
            total_size += ((frontier_node<DIMENSION> *) frontiers_)[i].pointer_->size();

        return total_size;
    }

    void density(density_array *array){
        for (uint16_t i = 0; i < num_nodes_; i++){
            uint8_t n_children = dfuds_->get_n_children(i);
            (*array)[n_children] = (*array)[n_children] + 1;
        }
        for (uint16_t i = 0; i < num_frontiers_; i++)
            ((frontier_node<DIMENSION> *) frontiers_)[i].pointer_->density(array);
    }

    void get_node_path(node_t node, symbol_t *node_path) {
        mutex.lock();
        if (node == 0){
            node_path[root_depth_] = dfuds_->next_symbol(0, 0, NUM_BRANCHES - 1);
            if (parent_tree_block_){
                mutex.unlock();
                parent_tree_block_->get_node_path(treeblock_frontier_num_, node_path);
            }
            else {
                mutex.unlock();
                parent_trie_node_->get_node_path_from_treeblock(root_depth_, node_path);
            }  
            return;          
        }

        preorder_t stack[35] = {};
        node_t path[35] = {};
        int symbol[35];
        size_t node_positions[2048];
        dfuds_->get_node_pos_bulk(node, node_positions);

        for (uint8_t i = 0; i < 35; i++){
            symbol[i] = -1;
        }
        preorder_t current_frontier = 0;
        int sTop = 0;

        // Todo: save path[sTop]
        node_t top_node = path[sTop];
        symbol[sTop] = dfuds_->next_symbol_with_node_pos(symbol[sTop] + 1, top_node, NUM_BRANCHES - 1, node_positions[top_node]);
        // symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, path[sTop], num_branches_ - 1);
        stack[sTop] = dfuds_->get_n_children_from_node_pos(0, node_positions[0]);
        // stack[sTop] = dfuds_->get_n_children(0);
        
        level_t current_level = root_depth_ + 1;
        node_t current_node = 1;

        if (frontiers_ != nullptr && current_frontier < num_frontiers_ && current_node > get_preorder(current_frontier))
            ++current_frontier;
        preorder_t next_frontier_preorder;

        if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
            next_frontier_preorder = -1;
        else
            next_frontier_preorder = get_preorder(current_frontier);

        while (current_node < num_nodes_ && sTop >= 0) {
            
            if (current_node == next_frontier_preorder) {
                if (current_node != node){
                    top_node = path[sTop];
                    symbol[sTop] = dfuds_->next_symbol_with_node_pos(symbol[sTop] + 1, top_node, NUM_BRANCHES - 1, node_positions[top_node]);
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
                stack[sTop] = dfuds_->get_n_children_from_node_pos(current_node, node_positions[current_node]);
                path[sTop] = current_node;

                symbol[sTop] = dfuds_->next_symbol_with_node_pos(symbol[sTop] + 1, current_node, NUM_BRANCHES - 1, node_positions[current_node]);
                ++current_level;
            }
            else if (current_level == max_depth_ - 1 && stack[sTop] > 1 && current_node < node)
            {
                top_node = path[sTop];
                symbol[sTop] = dfuds_->next_symbol_with_node_pos(symbol[sTop] + 1, top_node, NUM_BRANCHES - 1, node_positions[top_node]);
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
                symbol[sTop] = dfuds_->next_symbol_with_node_pos(symbol[sTop] + 1, top_node, NUM_BRANCHES - 1, node_positions[top_node]);
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
            mutex.unlock();
            parent_tree_block_->get_node_path(treeblock_frontier_num_, node_path);
        }
        else {
            mutex.unlock();
            parent_trie_node_->get_node_path_from_treeblock(root_depth_, node_path);
        }
        
    }

    symbol_t get_node_path_primary_key(n_leaves_t primary_key, symbol_t *node_path) {
        mutex.lock();

        preorder_t stack[35] = {};
        node_t path[35] = {};
        int symbol[35];
        for (uint8_t i = 0; i < 35; i++){
            symbol[i] = -1;
        }
        size_t node_positions[2048];  
        node_positions[0] = 0;

        int sTop = 0;
        node_t top_node = 0;
        symbol[sTop] = dfuds_->next_symbol_with_node_pos(symbol[sTop] + 1, 0, NUM_BRANCHES - 1, 0);
        stack[sTop] = dfuds_->get_n_children_from_node_pos(0, node_positions[0]);
          
        level_t current_level = root_depth_ + 1;
        node_t current_node = 1;

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
            
            dfuds_->get_node_data_pos_increment(current_node, node_positions); 

            if (current_node == next_frontier_preorder) {
                top_node = path[sTop];
                symbol[sTop] = dfuds_->next_symbol_with_node_pos(symbol[sTop] + 1, top_node, NUM_BRANCHES - 1, node_positions[top_node]);               
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
                stack[sTop] = dfuds_->get_n_children_from_node_pos(current_node, node_positions[current_node]);
                path[sTop] = current_node;

                symbol[sTop] = dfuds_->next_symbol_with_node_pos(symbol[sTop] + 1, current_node, NUM_BRANCHES - 1, node_positions[current_node]);
                ++current_level;
            }
            else
            {
                --stack[sTop];
                if (current_level == max_depth_ - 1){
                    
                    preorder_t new_current_primary = current_primary + dfuds_->get_n_children_from_node_pos(current_node, node_positions[current_node]);
                    bool found = false;

                    TimeStamp start = GetTimestamp(); 
                    for (preorder_t p = current_primary; p < new_current_primary; p ++)
                    {
                        if (primary_key_list[p].check_if_present(primary_key))
                        {
                            found = true;
                            // This optimization doesn't seem to be faster
                            parent_symbol = dfuds_->get_k_th_set_bit(current_node, p - current_primary /* 0-indexed*/, node_positions[current_node]);

                            break;                     
                        }
                    }
                    primary_time += GetTimestamp() - start;

                    current_primary = new_current_primary;

                    if (!found && stack[sTop] > 0){
                        top_node = path[sTop];

                        symbol[sTop] = dfuds_->next_symbol_with_node_pos(symbol[sTop] + 1, top_node, NUM_BRANCHES - 1, node_positions[top_node]);                  

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
                symbol[sTop] = dfuds_->next_symbol_with_node_pos(symbol[sTop] + 1, top_node, NUM_BRANCHES - 1, node_positions[top_node]);
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
            mutex.unlock();
            parent_tree_block_->get_node_path(treeblock_frontier_num_, node_path);
        }
        else {
            mutex.unlock();
            parent_trie_node_->get_node_path_from_treeblock(root_depth_, node_path);
        }        
        return parent_symbol;
    }

    data_point<DIMENSION> *node_path_to_coordinates(symbol_t *node_path){
        auto coordinates = new data_point<DIMENSION>();
        for (level_t i = 0; i < max_depth_; i++){
            symbol_t current_symbol = node_path[i];

            for (dimension_t j = 0; j < DIMENSION; j++){
                level_t current_bit = GETBIT(current_symbol, j);
                dimension_t coordinate_dimension = DIMENSION - 1 - j;
                point_t coordinate = coordinates->get_coordinate(coordinate_dimension);
                coordinate = (coordinate << 1) + current_bit;
                coordinates->set_coordinate(coordinate_dimension, coordinate);
            }
        }
        return coordinates;
    
    }

    void range_search_treeblock(data_point<DIMENSION> *start_range, data_point<DIMENSION> *end_range, tree_block *current_block,
                                            level_t level, preorder_t current_node, preorder_t prev_node, node_t current_frontier, preorder_t current_primary, point_array<DIMENSION> *found_points) {

        // mutex.lock();
        // // raise(SIGINT);
        // if (level == 29)
        //     raise(SIGINT);

        if (level == max_depth_) {

            // std::cout << "found" << std::endl;
            // GET which current primary corresponds to which node;
            // Now current_primary points to the leaf marked by tmp_symbol

            symbol_t parent_symbol = start_range->leaf_to_symbol(max_depth_ - 1, max_depth_);
            symbol_t tmp_symbol = dfuds_->next_symbol(0, prev_node, NUM_BRANCHES - 1);
            
            while (tmp_symbol != parent_symbol){    
                tmp_symbol = dfuds_->next_symbol(tmp_symbol + 1, prev_node, NUM_BRANCHES - 1);
                current_primary ++;
            }
            n_leaves_t list_size = primary_key_list[current_primary].size();

            total_leaf_number ++;
            for (n_leaves_t i = 0; i < list_size; i++)
            {
                auto primary_key = primary_key_list[current_primary].get(i);
 
                auto *leaf = new data_point<DIMENSION>();
                leaf->set(start_range->get());
                leaf->set_parent_treeblock(this);
                leaf->set_parent_node(prev_node);
                
                leaf->set_parent_symbol(parent_symbol);

                leaf->set_primary(primary_key);

                found_points->add_leaf(leaf);
            }
            // mutex.unlock();

            return;
        }
                                        
        if (current_node >= num_nodes_){
            // mutex.unlock();
            return;
        }

        if (num_frontiers() > 0 && current_frontier < num_frontiers() &&
            current_node == get_preorder(current_frontier)) {
            
            tree_block<DIMENSION, NUM_BRANCHES> *new_current_block = get_pointer(current_frontier);
            // mutex.unlock();
            node_t new_current_frontier = 0;
            preorder_t new_current_primary = 0;
            new_current_block->range_search_treeblock(start_range, end_range, new_current_block, level, 0, 0, new_current_frontier, new_current_primary, found_points); 
            return;
        }
        
        // TimeStamp start = GetTimestamp();

        symbol_t start_range_symbol = start_range->leaf_to_symbol(level, max_depth_);
        symbol_t end_range_symbol = end_range->leaf_to_symbol(level, max_depth_);
        representation_t representation = start_range_symbol ^ end_range_symbol;
        representation_t neg_representation = ~representation;

        struct data_point<DIMENSION> original_start_range = (*start_range);
        struct data_point<DIMENSION> original_end_range = (*end_range); 
        preorder_t new_current_node;
        tree_block *new_current_block;
        preorder_t new_current_frontier;
        preorder_t new_current_primary;

        symbol_t current_symbol = dfuds_->next_symbol(start_range_symbol, current_node, end_range_symbol);

        int count = 0;
        
        // bool has_child = false;

        // for (symbol_t i = current_symbol; i <= end_range_symbol; i++){

        //     if ((start_range_symbol & neg_representation) != (i & neg_representation))
        //         continue;

        //     if (dfuds_->has_symbol(current_node, i)){
        //         has_child = true;
        //     }            
            
        // }

        // if (!has_child)
        //     raise(SIGINT);

        while (current_symbol <= end_range_symbol){
            
            if (!dfuds_->has_symbol(current_node, current_symbol)){

                continue;
            }

            if ((start_range_symbol & neg_representation) == (current_symbol & neg_representation)){
                
                count ++;

                // if (count == 2)
                //     raise(SIGINT);

                new_current_block = current_block;
                new_current_frontier = current_frontier;
                new_current_primary = current_primary;

                TimeStamp start = GetTimestamp();
                new_current_node = current_block->child(new_current_block, current_node, current_symbol, level,
                                                        new_current_frontier, new_current_primary);

                child_latency += GetTimestamp() - start;
                start = GetTimestamp();
                start_range->update_range_morton(end_range, current_symbol, level, max_depth_); // NOT HERE
                update_range_latency += GetTimestamp() - start;

                if (new_current_block != current_block){

                    // mutex.unlock();
                    // raise(SIGINT);

                    new_current_block->range_search_treeblock(start_range, end_range, new_current_block, level + 1, new_current_node, 0, new_current_frontier, new_current_primary, found_points); // NOT HERE
                    // mutex.lock();
                }
                else {
                    // mutex.unlock();

                    current_block->range_search_treeblock(start_range, end_range, current_block, level + 1, new_current_node, current_node, new_current_frontier, new_current_primary, found_points);     // NOT HERE      
                    // mutex.lock();         
                }

                (*start_range) = original_start_range;
                (*end_range) = original_end_range;    
            }

            current_symbol = dfuds_->next_symbol(current_symbol + 1, current_node, end_range_symbol); // NOT HERE
        }

        // if (count == 0)
        //     std::cout << "Skipped " << " start range: " << start_range_symbol << " end range: " << end_range_symbol << " level: " << level << std::endl;
        //     // raise(SIGINT);

        // std::cout << "Treeblock count: " << count << " start range: " << start_range_symbol << " end range: " << end_range_symbol << " level: " << level << std::endl;
        // treeblock_range_search_latency += GetTimestamp() - start;
        // mutex.unlock();
    }
    
    void insert_primary_key_at_present_index(n_leaves_t index, n_leaves_t primary_key){

        mutex_p_key.lock();

        p_key_to_treeblock_compact.Set(primary_key, this);

        primary_key_list[index].push(primary_key);

        primary_key++;
        
        mutex_p_key.unlock();                

    }

    void insert_primary_key_at_index(n_leaves_t index, n_leaves_t primary_key){

        mutex_p_key.lock();

        p_key_to_treeblock_compact.Set(primary_key, this);
        
        primary_key_list.emplace(primary_key_list.begin() + index, bits::compact_ptr(primary_key));

        primary_key++;

        mutex_p_key.unlock();
    }

private:

    node_n_t max_tree_nodes_;
    level_t root_depth_{};
    node_n_t num_nodes_{};
    node_n_t tree_capacity_{};
    level_t max_depth_;
    compressed_bitmap::compressed_bitmap *dfuds_{};
    frontier_node<DIMENSION> *frontiers_ = nullptr; 
    node_n_t num_frontiers_ = 0;

    // TODO: parent_tree_block & parent_trie_node pointer can be combined
    tree_block *parent_tree_block_ = NULL;
    trie_node<DIMENSION, NUM_BRANCHES> *parent_trie_node_ = NULL;

    preorder_t treeblock_frontier_num_ = 0;

    std::vector<bits::compact_ptr> primary_key_list;
    std::shared_mutex mutex;
};

#endif //MD_TRIE_TREE_BLOCK_H
