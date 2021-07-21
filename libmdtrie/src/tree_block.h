#ifndef MD_TRIE_TREE_BLOCK_H
#define MD_TRIE_TREE_BLOCK_H

#include "point_array.h"
#include "trie_node.h"
#include "bitmap.h"
#include <sys/time.h>
#include <cmath>

uint64_t get_bit_count = 0;
uint64_t v2_storage_save_pos = 0;
uint64_t v2_storage_save_neg = 0;
uint64_t single_node_count = 0;
uint64_t total_number_nodes = 0;

template<dimension_t DIMENSION>
class tree_block {
public:

    tree_block *parent_tree_block_ = NULL;
    preorder_t treeblock_frontier_num_ = 0;
    trie_node<DIMENSION> *parent_trie_node = NULL;

    explicit tree_block(level_t root_depth, node_n_t tree_capacity, node_n_t num_nodes,
                        level_t max_depth, node_n_t max_tree_nodes) {
        root_depth_ = root_depth;
        tree_capacity_ = tree_capacity;
        num_branches_ = (symbol_t) pow(2, DIMENSION);
        max_depth_ = max_depth;
        max_tree_nodes_ = max_tree_nodes;
        num_nodes_ = num_nodes;
        dfuds_ = new bitmap::Bitmap(tree_capacity_ + 1, DIMENSION);
    }

    inline node_n_t num_frontiers() {
        return num_frontiers_;
    }

    inline tree_block *get_pointer(preorder_t current_frontier) const {
        return frontiers_[current_frontier].pointer_;
    }

    inline preorder_t get_preorder(preorder_t current_frontier) const 
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

    // This function selects the subTree starting from node 0
    // The selected subtree has the maximum subtree size
    node_t select_subtree(preorder_t &subtree_size, preorder_t &selected_node_depth) const {
        // index -> Number of children & preorder
        node_info index_to_node[4096];
        // index -> size of subtree & preorder
        subtree_info index_to_subtree[4096];
        // Index -> depth of the node
        preorder_t index_to_depth[4096];

        //  Corresponds to index_to_node, index_to_subtree, index_to_depth
        preorder_t node_stack_top = 0, subtree_stack_top = 0, depth_stack_top = 0;
        preorder_t depth;
        preorder_t current_frontier = 0;

        index_to_node[node_stack_top].preorder_ = 0;
        index_to_node[node_stack_top++].n_children_ = dfuds_->get_n_children(0);
        depth = root_depth_ + 1;
        preorder_t next_frontier_preorder;

        if (num_frontiers_ == 0 || current_frontier >= num_frontiers_)
            next_frontier_preorder = -1;
        else
            next_frontier_preorder = get_preorder(current_frontier);

        for (preorder_t i = 1; i < num_nodes_; ++i) {
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
            else
                --index_to_node[node_stack_top - 1].n_children_;
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
            if (num_nodes_ <= subtree_size_at_i * 4 && subtree_size_at_i * 4 <= 3 * num_nodes_ &&
                index_to_subtree[i].preorder_ < leftmost) {
                leftmost = min_node = index_to_subtree[i].preorder_;
                min_index = i;
            }
        }

        if (leftmost == (preorder_t) -1) {
            min_node = index_to_subtree[0].preorder_;
            if (num_nodes_ > 2 * index_to_subtree[0].subtree_size_) {
                min = num_nodes_ - 2 * index_to_subtree[0].subtree_size_;
            } else {
                min = 2 * index_to_subtree[0].subtree_size_ - num_nodes_;
            }
            min_index = 0;

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
        return min_node;
    }

    // This function inserts the string at the node position
    void insert(node_t node, data_point<DIMENSION> *leaf_point, level_t level, level_t length,
                            preorder_t current_frontier) {
        if (level == length) {
            return;
        }
        node_t original_node = node;
        uint64_t max_tree_nodes;
        if (root_depth_ <=/*=*/ 16) max_tree_nodes = 64;
        else if (root_depth_ <= 24) max_tree_nodes = 128;
        else max_tree_nodes = max_tree_nodes_;
        //  node is a frontier node
        if (frontiers_ != nullptr && current_frontier < num_frontiers_ && node == get_preorder(current_frontier)) {
            dfuds_->set_symbol(node, leaf_point->leaf_to_symbol(level, max_depth_), false);
            get_pointer(current_frontier)->insert(0, leaf_point, level, length, 0);

            return;
        }
            //  If there is only one character left
            //  Insert that character into the correct position
        else if (length == 1) {
            dfuds_->set_symbol(node, leaf_point->leaf_to_symbol(level, max_depth_), false);
            return;
        }
            // there is room in current block for new nodes
        else if (num_nodes_ + (length - level) - 1 <= tree_capacity_) {
            // skip_children_subtree returns the position under node where the new str[0] will be inserted
            symbol_t current_symbol = leaf_point->leaf_to_symbol(level, max_depth_);
            node = skip_children_subtree(node, current_symbol, level, current_frontier);

            // node_t dest_node = num_nodes_ + (length - level) - 2;
            dfuds_->set_symbol(original_node, current_symbol, false);
            node_t from_node = num_nodes_ - 1;

            //  In this while loop, we are making space for str
            //  By shifting nodes to the right of str[i] by len(str) spots

            if (from_node >= node) {
                // if (dfuds_->flag_size_ < num_nodes_ + length - level - 1){
                //     raise(SIGINT);
                // }
                dfuds_->shift_backward(node, length - level - 1);
                from_node = node;
                // dfuds_->bulk_clear_node(node, node + length - level - 2);
            }
            else {
                from_node++;
                dfuds_->bulk_clear_node(from_node, from_node + length - level - 2);
            }

            level++;
            //  Insert all remaining characters (Remember length -- above)
            
            
            for (level_t i = level; i < length; i++) {
                // dfuds_->clear_node(from_node);
                dfuds_->set_symbol(from_node, leaf_point->leaf_to_symbol(i, max_depth_), true);
                num_nodes_++;
                from_node++;
            }
            // shift the flags by length since all nodes have been shifted by that amount
            if (frontiers_ != nullptr)
                for (preorder_t j = current_frontier; j < num_frontiers_; ++j){
                    set_preorder(j, get_preorder(j) + length - level);
                    set_pointer(j, get_pointer(j));
                }

        } else if (num_nodes_ + (length - level) - 1 <= max_tree_nodes) {
            dfuds_->realloc_bitmap(num_nodes_ + length - level);
            tree_capacity_ = num_nodes_ + (length - level);
            insert(node, leaf_point, level, length, current_frontier);
        } else {
            preorder_t subtree_size, selected_node_depth;
            node_t selected_node = select_subtree(subtree_size, selected_node_depth);
            node_t orig_selected_node = selected_node;
            auto *new_dfuds = new bitmap::Bitmap(tree_capacity_ + 1, DIMENSION);

            preorder_t frontier;
            //  Find the first frontier node > selected_node
            for (frontier = 0; frontier < num_frontiers_; frontier++)
                if (get_preorder(frontier) > selected_node)
                    break;

            preorder_t frontier_selected_node = frontier;
            node_t insertion_node = node;

            node_t dest_node = 0;
            preorder_t n_nodes_copied = 0, copied_frontier = 0;

            bool insertion_in_new_block = false;
            bool is_in_root = false;

            preorder_t new_pointer_index = 0;

            frontier_node<DIMENSION> *new_pointer_array = nullptr;
            if (num_frontiers_ > 0) {
                new_pointer_array = (frontier_node<DIMENSION> *) malloc(sizeof(frontier_node<DIMENSION>) * (num_frontiers_ + 5));
            }
            preorder_t current_frontier_new_block = 0;

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
                }
                // If we see a frontier node, copy pointer to the new block
                if (new_pointer_array != nullptr && frontier < num_frontiers_ && selected_node == get_preorder(frontier)) {
                    new_pointer_array[new_pointer_index].preorder_ = dest_node;
                    new_pointer_array[new_pointer_index].pointer_ = get_pointer(frontier);
                    frontier++;
                    new_pointer_index++;
                    copied_frontier++;
                }
                // TODO: start here
                dfuds_->copy_node_cod(new_dfuds, selected_node, dest_node);

                selected_node += 1;
                dest_node += 1;
                n_nodes_copied += 1;
            }

            bool insertion_before_selected_tree = true;
            if (!insertion_in_new_block && frontier <= current_frontier)
                insertion_before_selected_tree = false;
            auto new_block = new tree_block(selected_node_depth, subtree_size, subtree_size, max_depth_, max_tree_nodes_);
            // Memory leak
            new_block->dfuds_ = new_dfuds;

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
            } else {
                //  If there are pointers copied to the new block
                new_pointer_array = (frontier_node<DIMENSION> *) realloc(new_pointer_array,
                                                            sizeof(frontier_node<DIMENSION>) * (new_pointer_index));

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
                frontiers_ = (frontier_node<DIMENSION> *) realloc(frontiers_, sizeof(frontier_node<DIMENSION>) * (num_frontiers_));
            }

            // Now, delete the subtree copied to the new block
            orig_selected_node++;

            // It seems that this optimization is not faster.
            if (selected_node < num_nodes_) {
                if (selected_node <= node && node < num_nodes_) {
                    insertion_node = node - selected_node + orig_selected_node;
                }
                dfuds_->shift_forward(selected_node, orig_selected_node);
            }

            if (subtree_size > length) {
                dfuds_->realloc_bitmap(tree_capacity_ - (subtree_size - length));
                tree_capacity_ -= subtree_size - length;
            } else {
                dfuds_->realloc_bitmap(tree_capacity_ - (subtree_size - 1));
                tree_capacity_ -= subtree_size - 1;
            }

            num_nodes_ -= (subtree_size - 1);

            if (!insertion_before_selected_tree)
                current_frontier -= copied_frontier;

            // If the insertion continues in the new block
            if (insertion_in_new_block) {
                if (is_in_root) {
                    dfuds_->set_symbol(insertion_node, leaf_point->leaf_to_symbol(level, max_depth_), true);
                    insert(insertion_node, leaf_point, level, length, current_frontier);
                } else {
                    new_block->insert(insertion_node, leaf_point, level, length, current_frontier_new_block);
                }
            }
            // If the insertion is in the old block
            else {
                insert(insertion_node, leaf_point, level, length, current_frontier);
            }
        }
    }

    // This function takes in a node (in preorder) and a symbol (branch index)
    // Return the child node (in preorder) designated by that symbol
    node_t skip_children_subtree(node_t node, symbol_t symbol, level_t current_level,
                                            preorder_t &current_frontier) const {

        // raise(SIGINT);
        if (current_level == max_depth_)
            return node;
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
            } else
                --stack[sTop];

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
                            preorder_t &current_frontier) const {
        get_bit_count ++;
        auto has_child = dfuds_->has_symbol(node, symbol);
        if (!has_child)
            return null_node;
        if (current_level == max_depth_)
            return node;

        node_t current_node;

        if (frontiers_ != nullptr && current_frontier < num_frontiers_ && node == get_preorder(current_frontier)) {
            p = get_pointer(current_frontier);
            current_frontier = 0;
            node_t temp_node = 0;
            current_node = p->skip_children_subtree(temp_node, symbol, current_level, current_frontier);
        } else
            current_node = skip_children_subtree(node, symbol, current_level, current_frontier);

        return current_node;
    }

    uint64_t size() const {

        uint64_t total_size = sizeof(level_t) * 1 + sizeof(node_n_t) * 4;
        total_size += sizeof(preorder_t);
        if (parent_tree_block_){
            total_size += sizeof(tree_block *);
        }
        if (parent_tree_block_){
            total_size += sizeof(trie_node<DIMENSION> *);
        }
        uint64_t v2_save_current_pos = 0;
        uint64_t v2_save_current_neg = 0;
        for (preorder_t i = 0; i < num_nodes_; i++){
            if (dfuds_->get_n_children(i) == 1){
                v2_storage_save_pos += num_branches_ - DIMENSION - 1;
                v2_save_current_pos += num_branches_ - DIMENSION - 1;
                single_node_count += 1;
            }
            else {
                v2_storage_save_neg += 1;
                v2_save_current_neg += 1;
            }
        }
        // if (dfuds_->size() + v2_save_current_pos != (tree_capacity_ + 1) * num_branches_ / 8 + sizeof(size_t) + v2_save_current_neg ){
        //     raise(SIGINT);
        // }
        total_number_nodes += num_nodes_;
        total_size += num_frontiers_ * (sizeof(preorder_t) + sizeof(tree_block *)) + sizeof(frontier_node<DIMENSION> *);
        total_size += dfuds_->size();

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
        
        if (node == 0){
            node_path[root_depth_] = dfuds_->next_symbol(0, 0, num_branches_ - 1);
            if (parent_tree_block_){
                parent_tree_block_->get_node_path(treeblock_frontier_num_, node_path);
            }
            else {
                parent_trie_node->get_node_path_from_treeblock(root_depth_, node_path);
            }  
            return;          
        }

        preorder_t stack[35] = {};
        node_t path[35] = {};
        int symbol[35];
        for (uint8_t i = 0; i < 35; i++){
            symbol[i] = -1;
        }
        preorder_t current_frontier = 0;
        // current_symbol, current_node, end_symbol_range (<= num_branches)
        int sTop = 0;
        symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, path[sTop], num_branches_ - 1);
        stack[sTop] = dfuds_->get_n_children(0);
        
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
                    symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, path[sTop], num_branches_ - 1);
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
                stack[sTop] = dfuds_->get_n_children(current_node);
                path[sTop] = current_node;

                // TODO: num_branches > 64
                symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, path[sTop], num_branches_ - 1);
                ++current_level;
            }
            else if (current_level == max_depth_ - 1 && stack[sTop] > 1 && current_node < node)
            {
                symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, path[sTop], num_branches_ - 1);
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
                symbol[sTop] = dfuds_->next_symbol(symbol[sTop] + 1, path[sTop], num_branches_ - 1);
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
            parent_trie_node->get_node_path_from_treeblock(root_depth_, node_path);
        }
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
                                            level_t level, preorder_t current_node, preorder_t prev_node, node_t current_frontier,
                                            point_array<DIMENSION> *found_points) {
        if (level == max_depth_) {
            auto *leaf = new data_point<DIMENSION>();
            leaf->set(start_range->get());
            leaf->set_parent_treeblock(this);
            leaf->set_parent_node(prev_node);
            leaf->set_parent_symbol(start_range->leaf_to_symbol(max_depth_ - 1, max_depth_));
            found_points->add_leaf(leaf);
            return;
        }
        
        symbol_t start_range_symbol = start_range->leaf_to_symbol(level, max_depth_);
        symbol_t end_range_symbol = end_range->leaf_to_symbol(level, max_depth_);
        representation_t representation = start_range_symbol ^ end_range_symbol;
        representation_t neg_representation = ~representation;

        struct data_point<DIMENSION> original_start_range = (*start_range);
        struct data_point<DIMENSION> original_end_range = (*end_range); 
        preorder_t new_current_node;
        tree_block *new_current_block;
        node_t new_current_frontier;

        symbol_t start_symbol_overlap = start_range_symbol & neg_representation;
        symbol_t current_symbol = dfuds_->next_symbol(start_range_symbol, current_node, end_range_symbol);

        while (current_symbol <= end_range_symbol){
            
            if (start_symbol_overlap == (current_symbol & neg_representation)){

                new_current_block = current_block;
                new_current_frontier = current_frontier;
                new_current_node = new_current_block->child(new_current_block, current_node, current_symbol, level,
                                                        new_current_frontier);

                start_range->update_range_morton(end_range, current_symbol, level, max_depth_);
                if (new_current_block != current_block){
                    new_current_block->range_search_treeblock(start_range, end_range, new_current_block, level + 1, new_current_node, new_current_node, new_current_frontier, found_points);
                }
                else {
                    new_current_block->range_search_treeblock(start_range, end_range, new_current_block, level + 1, new_current_node, current_node, new_current_frontier, found_points);                    
                }

                (*start_range) = original_start_range;
                (*end_range) = original_end_range;    
            }

            current_symbol = dfuds_->next_symbol(current_symbol + 1, current_node, end_range_symbol);
        }
    }

private:
    symbol_t num_branches_;
    node_n_t max_tree_nodes_;
    level_t root_depth_{};
    node_n_t num_nodes_{};
    node_n_t tree_capacity_{};
    level_t max_depth_;
    bitmap::Bitmap *dfuds_{};
    frontier_node<DIMENSION> *frontiers_ = nullptr; 
    node_n_t num_frontiers_ = 0;
};

#endif //MD_TRIE_TREE_BLOCK_H
