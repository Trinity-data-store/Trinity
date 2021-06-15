#include "tree_block.h"

uint64_t dfuds_size = 0;

void tree_block::copy_node_cod(bitmap::Bitmap *from_dfuds, bitmap::Bitmap *to_dfuds, node_t from, node_t to,
                               symbol_t n_branches, symbol_t width) {
    symbol_t visited = 0;
    while (visited < width) {
        if (width - visited > 64) {
            to_dfuds->SetValPos(to * n_branches + visited, from_dfuds->GetValPos(from * n_branches + visited, 64), 64);
            visited += 64;
        } else {
            symbol_t left = width - visited;
            to_dfuds->SetValPos(to * n_branches + visited, from_dfuds->GetValPos(from * n_branches + visited, left),
                                left);
            break;
        }
    }
}

tree_block *tree_block::get_pointer(preorder_t current_frontier) const {
    return ((frontier_node *) frontiers_)[current_frontier].pointer_;
}

preorder_t tree_block::get_preorder(preorder_t current_frontier) const {
    return ((frontier_node *) frontiers_)[current_frontier].preorder_;
}

void tree_block::set_preorder(preorder_t current_frontier, preorder_t preorder) {
    ((frontier_node *) frontiers_)[current_frontier].preorder_ = preorder;
}

void tree_block::set_pointer(preorder_t current_frontier, tree_block *pointer) {
    ((frontier_node *) frontiers_)[current_frontier].pointer_ = pointer;
}

preorder_t tree_block::get_n_children(node_t node, symbol_t n_branches) const {
    return dfuds_->popcount(node * n_branches, n_branches);
}

preorder_t tree_block::get_child_skip(node_t node, symbol_t col, symbol_t n_branches) const {
    return dfuds_->popcount(node * n_branches, col);
}

// This function selects the subTree starting from node 0
// The selected subtree has the maximum subtree size
node_t tree_block::select_subtree(preorder_t &subtree_size, preorder_t &selected_node_depth) const {
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
    index_to_node[node_stack_top++].n_children_ = get_n_children(0, num_branches_);
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
            index_to_node[node_stack_top++].n_children_ = get_n_children(i, num_branches_);
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
    // TODO(anuragk): Initialize appropriately
    preorder_t min_node, min, min_index;
    preorder_t diff;

    auto leftmost = (preorder_t) -1;

    for (preorder_t i = 0; i < subtree_stack_top; ++i) {
//       if (lowerB <= subtrees[i].subtreeSize && subtrees[i].subtreeSize <= upperB && subtrees[i].preorder < leftmost) {
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
void tree_block::insert(node_t node, data_point *leaf_point, level_t level, level_t length,
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
        dfuds_->SetBit(node * num_branches_ + leaf_point->leaf_to_symbol(level, max_depth_));
        get_pointer(current_frontier)->insert(0, leaf_point, level, length, 0);

        return;
    }
        //  If there is only one character left
        //  Insert that character into the correct position
    else if (length == 1) {
        dfuds_->SetBit(node * num_branches_ + leaf_point->leaf_to_symbol(level, max_depth_));
        return;
    }
        // there is room in current block for new nodes
    else if (num_nodes_ + (length - level) - 1 <= tree_capacity_) {
        // skip_children_subtree returns the position under node where the new str[0] will be inserted
        symbol_t current_symbol = leaf_point->leaf_to_symbol(level, max_depth_);
        node = skip_children_subtree(node, current_symbol, level, current_frontier);

        node_t dest_node = num_nodes_ + (length - level) - 2;
        node_t from_node = num_nodes_ - 1;

        //  In this while loop, we are making space for str
        //  By shifting nodes to the right of str[i] by len(str) spots

        // dfuds_->BulkCopy(from_node * n_branches_, node * n_branches_, dest_node * n_branches_);
        if (from_node >= node) {
            dfuds_->BulkCopy_backward((from_node + 1) * num_branches_, (dest_node + 1) * num_branches_,
                                      (from_node - node + 1) * num_branches_);
            // TODO(anuragk): This line is never used
            dest_node -= from_node - (node - 1);
            from_node = node - 1;
        }
        // while (from_node >= node)
        // {
        //     copy_node_cod(dfuds_, dfuds_, from_node, dest_node, n_branches_, n_branches_);
        //     dest_node--;
        //     from_node--;
        // }

        dfuds_->SetBit(original_node * num_branches_ + current_symbol);
        level++;
        from_node++;
        //  Insert all remaining characters (Remember length -- above)
        for (level_t i = level; i < length; i++) {
            dfuds_->ClearWidth(from_node * num_branches_, num_branches_);
            dfuds_->SetBit(from_node * num_branches_ + leaf_point->leaf_to_symbol(i, max_depth_));
            num_nodes_++;
            from_node++;
        }
        // shift the flags by length since all nodes have been shifted by that amount
        if (frontiers_ != nullptr)
            for (preorder_t j = current_frontier; j < num_frontiers_; ++j)
                set_preorder(j, get_preorder(j) + length - level);
    } else if (num_nodes_ + (length - level) - 1 <= max_tree_nodes) {
        dfuds_->Realloc((num_nodes_ + (length - level)) * num_branches_);
        tree_capacity_ = num_nodes_ + (length - level);
        insert(node, leaf_point, level, length, current_frontier);
    } else {
        preorder_t subtree_size, selected_node_depth;
        node_t selected_node = select_subtree(subtree_size, selected_node_depth);
        node_t orig_selected_node = selected_node;
        auto *new_dfuds = new bitmap::Bitmap((tree_capacity_ + 1) * num_branches_);

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

        frontier_node *new_pointer_array = nullptr;
        if (num_frontiers_ > 0) {
            new_pointer_array = (frontier_node *) malloc(sizeof(frontier_node) * (num_frontiers_ + 5));
        }
        preorder_t current_frontier_new_block = 0;

        //  Copy all nodes of the subtree to the new block
        // This optimization doesn't seem to work
        // if (n_nodes_copied < subtree_size){
        //     copy_node_cod(dfuds_, new_dfuds, selected_node, dest_node, n_branches_, subtree_size * n_branches_);
        // }

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
            copy_node_cod(dfuds_, new_dfuds, selected_node, dest_node, num_branches_, num_branches_);

            selected_node += 1;
            dest_node += 1;
            n_nodes_copied += 1;
        }

        bool insertion_before_selected_tree = true;
        if (!insertion_in_new_block && frontier <= current_frontier)
            insertion_before_selected_tree = false;
        auto new_block = new tree_block(dimensions_, selected_node_depth, subtree_size, subtree_size, max_depth_);
        // treeblock *new_block = create_new_treeblock(selected_node_depth, subtree_size, subtree_size, dimensions_, max_depth_);
        // Memory leak
        new_block->dfuds_ = new_dfuds;

        //  If no pointer is copied to the new block
        if (new_pointer_index == 0) {
            if (new_pointer_array != nullptr)
                free(new_pointer_array);

            // Expand frontiers array to add one more frontier node
            frontiers_ = realloc(frontiers_, sizeof(frontier_node) * (num_frontiers_ + 1));
            // Shift right one spot to move the pointers from flagSelectedNode + 1 to nPtrs
            for (preorder_t j = num_frontiers_; j > frontier_selected_node; --j) {
                set_pointer(j, get_pointer(j - 1));
                set_preorder(j, get_preorder(j - 1) - subtree_size + 1);
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
            set_preorder(frontier_selected_node, orig_selected_node);
            set_pointer(frontier_selected_node, new_block);

            for (preorder_t j = frontier_selected_node + 1; frontier < num_frontiers_; j++, frontier++) {
                set_pointer(j, get_pointer(frontier));
                set_preorder(j, get_preorder(frontier) - subtree_size + 1);
            }
            num_frontiers_ = num_frontiers_ - copied_frontier + 1;
            frontiers_ = realloc(frontiers_, sizeof(frontier_node) * (num_frontiers_));
        }

        // Now, delete the subtree copied to the new block
        // TODO(anuragk): This line is never used
        frontier = frontier_selected_node + 1;
        orig_selected_node++;

        // It seems that this optimization is not faster.
        if (selected_node < num_nodes_) {
            if (selected_node <= node && node < num_nodes_) {
                insertion_node = node - selected_node + orig_selected_node;
            }
            dfuds_->BulkCopy_forward(selected_node * num_branches_, orig_selected_node * num_branches_,
                                     num_branches_ * (num_nodes_ - selected_node));

            // TODO(anuragk): These two lines are never used
            orig_selected_node += num_nodes_ - selected_node;
            selected_node = num_nodes_;
        }

        // while (selected_node < n_nodes_)
        // {
        // //     // selected_node is the immediate node after the copied block
        // //     // orig_selected_node is the original node where we want to turn into a frontier node
        // //     // node is the node where we want to insert the symbol str[0]
        //     copy_node_cod(dfuds_, dfuds_, selected_node, orig_selected_node, n_branches_, n_branches_);
        //     if (selected_node == node)
        //         insertion_node = orig_selected_node;
        //     selected_node++;
        //     orig_selected_node++;
        // }

        if (subtree_size > length) {
            dfuds_->Realloc((tree_capacity_ - (subtree_size - length)) * num_branches_);
            tree_capacity_ -= subtree_size - length;
        } else {
            dfuds_->Realloc((tree_capacity_ - (subtree_size - 1)) * num_branches_);
            tree_capacity_ -= subtree_size - 1;
        }
        num_nodes_ -= (subtree_size - 1);

        if (!insertion_before_selected_tree)
            current_frontier -= copied_frontier;

        // If the insertion continues in the new block
        if (insertion_in_new_block) {
            if (is_in_root) {
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
node_t tree_block::skip_children_subtree(node_t &node, symbol_t symbol, level_t current_level,
                                         preorder_t &current_frontier) const {
    if (current_level == max_depth_)
        return node;
    int sTop = -1;
    preorder_t n_children_skip = get_child_skip(node, symbol, num_branches_);
    preorder_t n_children = get_n_children(node, num_branches_);
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
            stack[++sTop] = get_n_children(current_node, num_branches_);
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
node_t tree_block::child(tree_block *&p, node_t &node, symbol_t symbol, level_t &current_level,
                         preorder_t &current_frontier) const {
    auto has_child = dfuds_->GetBit(node * num_branches_ + symbol);
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

uint64_t tree_block::size() const {

    // Can be hard coded as global variable:
    // preorder_type max_tree_nodes_;
    // level_type max_depth_;
    // uint8_t dimensions_;
    // symbol_type n_branches_;

    uint64_t total_size = sizeof(level_t) * 1 + sizeof(node_n_t) * 4;
    total_size += num_frontiers_ * (sizeof(preorder_t) + sizeof(tree_block *)) + sizeof(frontier_node *);
    total_size += dfuds_->size();
    dfuds_size += dfuds_->size();
    for (uint16_t i = 0; i < num_frontiers_; i++)
        total_size += ((frontier_node *) frontiers_)[i].pointer_->size();

    return total_size;
}

void tree_block::range_search_treeblock(data_point *start_range, data_point *end_range, tree_block *current_block,
                                        level_t level, preorder_t current_node, node_t current_frontier,
                                        point_array *found_points, uint8_t *representation) {
    if (level == max_depth_) {
        auto *leaf = new data_point(dimensions_);

        for (uint8_t j = 0; j < dimensions_; j++) {
            point_t start_coordinate = start_range->coordinates[j];
            leaf->coordinates[j] = start_coordinate;
        }
        found_points->add_leaf(leaf);
        return;
    }

    start_range->get_representation(end_range, representation, level, max_depth_);
    range_traverse_treeblock(start_range, end_range, representation, 0, current_block, level, current_node,
                             current_frontier, found_points);
}

void tree_block::range_traverse_treeblock(data_point *start_range, data_point *end_range, uint8_t representation[],
                                          uint8_t index, tree_block *current_block, level_t level,
                                          preorder_t current_node, node_t current_frontier,
                                          point_array *found_points) {
    if (index == dimensions_) {

        start_range->update_range(end_range, representation, level, max_depth_);

        symbol_t current_symbol = 0;
        for (uint8_t j = 0; j < dimensions_; j++) {
            current_symbol = current_symbol << 1U;
            if (representation[j] == 1) {
                current_symbol += 1;
            }
        }

        node_t temp_node = current_block->child(current_block, current_node, current_symbol, level,
                                                current_frontier);

        if (temp_node == (node_t) -1)
            return;
        current_node = temp_node;

        if (current_block->num_frontiers() > 0 && current_frontier < current_block->num_frontiers() &&
            current_node == current_block->get_preorder(current_frontier)) {
            current_block = current_block->get_pointer(current_frontier);
            current_node = (node_t) 0;
            current_frontier = 0;
        }
        range_search_treeblock(start_range, end_range, current_block, level + 1, current_node, current_frontier,
                               found_points, representation);

        return;
    }
    if (representation[index] == 2) {
        coordinates_t original_start_coordinates(start_range->coordinates);
        coordinates_t original_end_coordinates(end_range->coordinates);

        representation[index] = 0;
        range_traverse_treeblock(start_range, end_range, representation, index + 1, current_block, level, current_node,
                                 current_frontier, found_points);

        start_range->coordinates = original_start_coordinates;
        end_range->coordinates = original_end_coordinates;

        representation[index] = 1;
        range_traverse_treeblock(start_range, end_range, representation, index + 1, current_block, level, current_node,
                                 current_frontier, found_points);
    } else {
        range_traverse_treeblock(start_range, end_range, representation, index + 1, current_block, level, current_node,
                                 current_frontier, found_points);
    }
}
