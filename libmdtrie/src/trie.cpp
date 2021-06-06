#include "trie.h"

void copy_node_cod(bitmap::Bitmap *from_dfuds, bitmap::Bitmap *to_dfuds, node_type from, node_type to, symbol_type n_branches)
{ 
    symbol_type visited = 0;
    while (visited < n_branches)
    {
        if (n_branches - visited > 64)
        {
            to_dfuds->SetValPos(to * n_branches + visited, from_dfuds->GetValPos(from * n_branches + visited, 64), 64);
            visited += 64;
        }
        else
        {
            symbol_type left = n_branches - visited;
            to_dfuds->SetValPos(to * n_branches + visited, from_dfuds->GetValPos(from * n_branches + visited, left), left);
            break;
        }
    }
}

treeblock *treeblock::get_pointer(preorder_type current_frontier)
{
    return ((frontier_node *)frontiers_)[current_frontier].pointer_;
}

preorder_type treeblock::get_preorder(preorder_type current_frontier)
{
    return ((frontier_node *)frontiers_)[current_frontier].preorder_;
}

void treeblock::set_preorder(preorder_type current_frontier, preorder_type preorder)
{
    ((frontier_node *)frontiers_)[current_frontier].preorder_ = preorder;
}

void treeblock::set_pointer(preorder_type current_frontier, treeblock *pointer)
{
    ((frontier_node *)frontiers_)[current_frontier].pointer_ = pointer;
}

preorder_type get_n_children(bitmap::Bitmap *dfuds, node_type node, symbol_type n_branches)
{

    return dfuds->popcount(node * n_branches, n_branches);
}

preorder_type get_child_skip(bitmap::Bitmap *dfuds, node_type node, symbol_type col, symbol_type n_branches)
{
    return dfuds->popcount(node * n_branches, col);
}

treeblock *create_new_treeblock(level_type root_depth, preorder_type n_nodes, preorder_type max_nodes, int dimensions, level_type max_depth)
{
    symbol_type n_branches = pow(2, dimensions);
    treeblock *current_treeblock = new treeblock(dimensions, max_depth);

    current_treeblock->dfuds_ = new bitmap::Bitmap((max_nodes + 1) * n_branches);
    current_treeblock->root_depth_ = root_depth;
    current_treeblock->n_nodes_ = n_nodes;
    current_treeblock->frontiers_ = NULL;
    current_treeblock->n_frontiers_ = 0;
    current_treeblock->max_nodes_ = max_nodes;
    return current_treeblock;
}

trie_node *md_trie::create_new_trie_node()
{
    trie_node *current_trie_node = new trie_node(n_branches_);
    for (symbol_type i = 0; i < n_branches_; i++)
        current_trie_node->children_[i] = NULL;

    current_trie_node->block = NULL;
    return current_trie_node;
}

// This function selectes the subTree starting from node 0
// The selected subtree has the maximum subtree size
node_type treeblock::select_subtree(preorder_type &subtree_size, preorder_type &selected_node_depth)
{
    // index -> Number of children & preorder
    node_info index_to_node[4096];
    // index -> size of subtree & preorder
    subtree_info index_to_subtree[4096];
    // Index -> depth of the node
    preorder_type index_to_depth[4096];

    //  Corresponds to index_to_node, index_to_subtree, index_to_depth
    preorder_type node_stack_top = 0, subtree_stack_top = 0, depth_stack_top = 0;
    node_type node = 0;
    preorder_type depth;
    preorder_type current_frontier = 0;

    index_to_node[node_stack_top].preorder_ = 0;
    index_to_node[node_stack_top++].n_children_ = get_n_children(dfuds_, 0, n_branches_);
    node++;
    depth = root_depth_ + 1;
    preorder_type next_frontier_preorder;

    if (n_frontiers_ == 0 || current_frontier >= n_frontiers_)
        next_frontier_preorder = -1;
    else
        next_frontier_preorder = get_preorder(current_frontier);

    for (preorder_type i = 1; i < n_nodes_; ++i)
    {
        // If meet a frontier node
        if (i == next_frontier_preorder)
        {
            ++current_frontier;
            if (n_frontiers_ == 0 || current_frontier >= n_frontiers_)
                next_frontier_preorder = -1;
            else
                next_frontier_preorder = get_preorder(current_frontier);
            --index_to_node[node_stack_top - 1].n_children_;
        }
        //  Start searching for its children
        else if (depth < max_depth_ - 1)
        {
            index_to_node[node_stack_top].preorder_ = i;
            index_to_node[node_stack_top++].n_children_ = get_n_children(dfuds_, i, n_branches_);
            depth++;
        }
        //  Reached the maxDepth level
        else
            --index_to_node[node_stack_top - 1].n_children_;
        while (node_stack_top > 0 && index_to_node[node_stack_top - 1].n_children_ == 0)
        {
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
    preorder_type min_node, min, min_index;
    preorder_type diff;

    min_node = index_to_subtree[0].preorder_,
    min = n_nodes_ - 2 * index_to_subtree[0].subtree_size_;
    min_index = 0;

    for (preorder_type i = 1; i < subtree_stack_top; ++i)
    {
        if (n_nodes_ > 2 * index_to_subtree[i].subtree_size_)
        {
            diff = n_nodes_ - 2 * index_to_subtree[i].subtree_size_;
        }
        else
        {
            diff = 2 * index_to_subtree[i].subtree_size_ - n_nodes_;
        }
        if (diff < min)
        {
            min = diff;
            min_node = index_to_subtree[i].preorder_;
            min_index = i;
        }
    }
    subtree_size = index_to_subtree[min_index].subtree_size_;
    selected_node_depth = index_to_depth[min_index];
    return min_node;
}

// This function inserts the string at the node position
void treeblock::insert(node_type node, leaf_config *leaf_point, level_type level, level_type length, preorder_type current_frontier)
{
    if (level == length)
    {
        return;
    }
    node_type original_node = node;

    //  node is a frontier node
    if (frontiers_ != NULL && current_frontier < n_frontiers_ && node == get_preorder(current_frontier))
    {
        dfuds_->SetBit(node * n_branches_ + leaf_to_symbol(leaf_point, level, dimensions_, max_depth_));
        get_pointer(current_frontier)->insert(0, leaf_point, level, length, 0);

        return;
    }
    //  If there is only one character left
    //  Insert that character into the correct position
    else if (length == 1)
    {
        dfuds_->SetBit(node * n_branches_ + leaf_to_symbol(leaf_point, level, dimensions_, max_depth_));
        return;
    }
    // there is room in current block for new nodes
    else if (n_nodes_ + (length - level) - 1 <= max_nodes_)
    {
        // skip_children_subtree returns the position under node where the new str[0] will be inserted
        node = skip_children_subtree(node, leaf_to_symbol(leaf_point, level, dimensions_, max_depth_), level, current_frontier);

        node_type dest_node = n_nodes_ + (length - level) - 2;
        node_type from_node = n_nodes_ - 1;

        //  In this while loop, we are making space for str
        //  By shifting nodes to the right of str[i] by len(str) spots
        while (from_node >= node)
        {
            copy_node_cod(dfuds_, dfuds_, from_node, dest_node, n_branches_);
            dest_node--;
            from_node--;
        }

        dfuds_->SetBit(original_node * n_branches_ + leaf_to_symbol(leaf_point, level, dimensions_, max_depth_));
        level++;
        from_node++;
        //  Insert all remaining characters (Remember length -- above)
        for (level_type i = level; i < length; i++)
        {
            dfuds_->ClearWidth(from_node * n_branches_, n_branches_);
            dfuds_->SetBit(from_node * n_branches_ + leaf_to_symbol(leaf_point, i, dimensions_, max_depth_));
            n_nodes_++;
            from_node++;
        }
        // shift the flags by length since all nodes have been shifted by that amount
        if (frontiers_ != NULL)
            for (preorder_type j = current_frontier; j < n_frontiers_; ++j)
                set_preorder(j, get_preorder(j) + length - level);
    }
    else
    {
        preorder_type subtree_size, selected_node_depth;
        node_type selected_node = select_subtree(subtree_size, selected_node_depth);
        node_type orig_selected_node = selected_node;
        bitmap::Bitmap *new_dfuds = new bitmap::Bitmap((max_nodes_ + 1) * n_branches_);

        preorder_type frontier;
        //  Find the first frontier node > selected_node
        for (frontier = 0; frontier < n_frontiers_; frontier++)
            if (get_preorder(frontier) > selected_node)
                break;

        preorder_type frontier_selected_node = frontier;
        node_type insertion_node = node;

        node_type dest_node = 0;
        preorder_type n_nodes_copied = 0, copied_frontier = 0;

        bool insertion_in_new_block = false;
        bool is_in_root = false;

        preorder_type new_pointer_index = 0;

        frontier_node *new_pointer_array = NULL;
        if (n_frontiers_ > 0)
        {
            new_pointer_array = (frontier_node *)malloc(sizeof(frontier_node) * (n_frontiers_ + 5));
        }
        preorder_type current_frontier_new_block = 0;

        //  Copy all nodes of the subtree to the new block
        while (n_nodes_copied < subtree_size)
        {
            //  If we meet the current node (from which we want to do insertion)
            // insertion_node is the new preorder in new block where we want to insert a node
            if (selected_node == node)
            {
                insertion_in_new_block = true;
                if (dest_node != 0)
                    insertion_node = dest_node;
                else
                {
                    insertion_node = node;
                    is_in_root = true;
                }
                current_frontier_new_block = copied_frontier;
            }
            // If we see a frontier node, copy pointer to the new block
            if (new_pointer_array != NULL && frontier < n_frontiers_ && selected_node == get_preorder(frontier))
            {
                new_pointer_array[new_pointer_index].preorder_ = dest_node;
                new_pointer_array[new_pointer_index].pointer_ = get_pointer(frontier);
                frontier++;
                new_pointer_index++;
                copied_frontier++;
            }
            copy_node_cod(dfuds_, new_dfuds, selected_node, dest_node, n_branches_);

            selected_node += 1;
            dest_node += 1;
            n_nodes_copied += 1;
        }

        bool insertion_before_selected_tree = true;
        if (!insertion_in_new_block && frontier <= current_frontier)
            insertion_before_selected_tree = false;

        treeblock *new_block = create_new_treeblock(selected_node_depth, subtree_size, max_tree_nodes, dimensions_, max_depth_);
        // Memory leak
        new_block->dfuds_ = new_dfuds;

        //  If no pointer is copied to the new block
        if (new_pointer_index == 0)
        {
            if (new_pointer_array != NULL)
                free(new_pointer_array);

            // Expand frontiers array to add one more frontier node
            frontiers_ = realloc(frontiers_, sizeof(frontier_node) * (n_frontiers_ + 1));
            // Shift right one spot to move the pointers from flagSelectedNode + 1 to nPtrs
            for (preorder_type j = n_frontiers_; j > frontier_selected_node; --j)
            {
                set_pointer(j, get_pointer(j - 1));
                set_preorder(j, get_preorder(j - 1) - subtree_size + 1);
            }
            //  Insert that new frontier node
            set_preorder(frontier_selected_node, orig_selected_node);
            set_pointer(frontier_selected_node, new_block);
            n_frontiers_++;
        }
        else
        {
            //  If there are pointers copied to the new block
            new_pointer_array = (frontier_node *)realloc(new_pointer_array, sizeof(frontier_node) * (new_pointer_index));

            new_block->frontiers_ = new_pointer_array;
            new_block->n_frontiers_ = new_pointer_index;
            set_preorder(frontier_selected_node, orig_selected_node);
            set_pointer(frontier_selected_node, new_block);

            for (preorder_type j = frontier_selected_node + 1; frontier < n_frontiers_; j++, frontier++)
            {
                set_pointer(j, get_pointer(frontier));
                set_preorder(j, get_preorder(frontier) - subtree_size + 1);
            }
            n_frontiers_ = n_frontiers_ - copied_frontier + 1;
            frontiers_ = realloc(frontiers_, sizeof(frontier_node) * (n_frontiers_));
        }
        // Now, delete the subtree copied to the new block
        frontier = frontier_selected_node + 1;
        orig_selected_node++;

        while (selected_node < n_nodes_)
        {
            // selected_node is the immediate node after the copied block
            // orig_selected_node is the original node where we want to turn into a frontier node
            // node is the node where we want to insert the symbol str[0]
            copy_node_cod(dfuds_, dfuds_, selected_node, orig_selected_node, n_branches_);
            if (selected_node == node)
                insertion_node = orig_selected_node;
            selected_node++;
            orig_selected_node++;
        }
        n_nodes_ -= (subtree_size - 1);

        if (!insertion_before_selected_tree)
            current_frontier -= copied_frontier;

        // If the insertion continues in the new block
        if (insertion_in_new_block)
        {
            if (is_in_root)
            {
                insert(insertion_node, leaf_point, level, length, current_frontier);
            }
            else
            {
                new_block->insert(insertion_node, leaf_point, level, length, current_frontier_new_block);
            }
        }
        // If the insertion is in the old block
        else
        {
            insert(insertion_node, leaf_point, level, length, current_frontier);
        }
    }
}

// This function takes in a node (in preorder) and a symbol (branch index)
// Return the child node (in preorder) designated by that symbol
node_type treeblock::skip_children_subtree(node_type &node, symbol_type symbol, level_type current_level, preorder_type &current_frontier)
{
    if (current_level == max_depth_)
        return node;
    int sTop = -1;
    preorder_type n_children_skip = get_child_skip(dfuds_, node, symbol, n_branches_);
    preorder_type n_children = get_n_children(dfuds_, node, n_branches_);
    preorder_type diff = n_children - n_children_skip;
    preorder_type stack[100];
    stack[++sTop] = n_children;

    node_type current_node = node + 1;

    if (frontiers_ != NULL && current_frontier < n_frontiers_ && current_node > get_preorder(current_frontier))
        ++current_frontier;
    preorder_type next_frontier_preorder;

    if (n_frontiers_ == 0 || current_frontier >= n_frontiers_)
        next_frontier_preorder = -1;
    else
        next_frontier_preorder = get_preorder(current_frontier);

    ++current_level;
    while (current_node < n_nodes_ && sTop >= 0 && diff < stack[0])
    {
        if (current_node == next_frontier_preorder)
        {
            ++current_frontier;
            if (n_frontiers_ == 0 || current_frontier >= n_frontiers_)
                next_frontier_preorder = -1;
            else
                next_frontier_preorder = get_preorder(current_frontier);
            --stack[sTop];
        }
        // It is "-1" because current_level is 0th indexed.
        else if (current_level < max_depth_ - 1)
        {
            stack[++sTop] = get_n_children(dfuds_, current_node, n_branches_);
            ++current_level;
        }
        else
            --stack[sTop];

        ++current_node;
        while (sTop >= 0 && stack[sTop] == 0)
        {
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
node_type treeblock::child(treeblock *&p, node_type &node, symbol_type symbol, level_type &current_level, preorder_type &current_frontier)
{
    bool has_child = (bool)dfuds_->GetBit(node * n_branches_ + symbol);
    if (!has_child)
        return null_node;
    if (current_level == max_depth_ && has_child)
        return node;

    node_type current_node;

    if (frontiers_ != NULL && current_frontier < n_frontiers_ && node == get_preorder(current_frontier))
    {
        p = get_pointer(current_frontier);
        current_frontier = 0;
        node_type temp_node = 0;
        current_node = p->skip_children_subtree(temp_node, symbol, current_level, current_frontier);
    }
    else
        current_node = skip_children_subtree(node, symbol, current_level, current_frontier);

    return current_node;
}

// Traverse the current TreeBlock, going into frontier nodes as needed
// Until it cannot traverse further and calls insertion
void md_trie::insert_remaining(treeblock *root, leaf_config *leaf_point, level_type length, level_type level)
{
    treeblock *current_block = root;
    node_type current_node = 0;
    preorder_type current_frontier = 0;

    node_type temp_node = 0;
    while (level < length)
    {
        temp_node = current_block->child(current_block, current_node, leaf_to_symbol(leaf_point, level, dimensions_, max_depth_), level, current_frontier);
        if (temp_node == (node_type)-1)
            break;
        current_node = temp_node;
        if (current_block->n_frontiers_ > 0 && current_frontier < current_block->n_frontiers_ && current_node == current_block->get_preorder(current_frontier))
        {
            current_block = current_block->get_pointer(current_frontier);
            current_node = (node_type)0;
            current_frontier = 0;
        }
        level++;
    }
    current_block->insert(current_node, leaf_point, level, length, current_frontier);
}

// This function is used for testing.
// It differs from above as it only returns True or False.
bool md_trie::walk_treeblock(treeblock *current_block, leaf_config *leaf_point, level_type length, level_type level)
{

    preorder_type current_frontier = 0;
    node_type current_node = 0;
    node_type temp_node = 0;
    while (level < length)
    {
        temp_node = current_block->child(current_block, current_node, leaf_to_symbol(leaf_point, level, dimensions_, max_depth_), level, current_frontier);
        if (temp_node == (node_type)-1)
            return false;
        current_node = temp_node;

        if (current_block->n_frontiers_ > 0 && current_frontier < current_block->n_frontiers_ && current_node == current_block->get_preorder(current_frontier))
        {
            current_block = current_block->get_pointer(current_frontier);
            current_node = (node_type)0;
            current_frontier = 0;
        }
        level++;
    }
    return true;
}

// This function goes down the trie
// Return the treeblock at the leaf of the trie
treeblock *md_trie::walk_trie(trie_node *current_trie_node, leaf_config *leaf_point, level_type &level)
{

    while (current_trie_node->children_[leaf_to_symbol(leaf_point, level, dimensions_, max_depth_)])
        current_trie_node = current_trie_node->children_[leaf_to_symbol(leaf_point, level++, dimensions_, max_depth_)];
    while (level < trie_depth)
    {
        current_trie_node->children_[leaf_to_symbol(leaf_point, level, dimensions_, max_depth_)] = create_new_trie_node();
        current_trie_node = current_trie_node->children_[leaf_to_symbol(leaf_point, level, dimensions_, max_depth_)];
        level++;
    }
    treeblock *current_treeblock = NULL;
    if (current_trie_node->block == NULL)
    {
        current_treeblock = create_new_treeblock(trie_depth, 1, max_tree_nodes, dimensions_, max_depth_);
        current_trie_node->block = current_treeblock;
    }
    else
        current_treeblock = (treeblock *)current_trie_node->block;
    return current_treeblock;
}

// This function inserts a string into a trie_node.
// The first part it traverses is the trie, followed by traversing the treeblock
void md_trie::insert_trie(leaf_config *leaf_point, level_type length)
{
    if (root_ == NULL)
    {
        root_ = create_new_trie_node();
    }
    level_type level = 0;
    trie_node *current_trie_node = root_;
    treeblock *current_treeblock = walk_trie(current_trie_node, leaf_point, level);
    insert_remaining(current_treeblock, leaf_point, length, level);
}

// Given the leaf_point and the level we are at, return the Morton code corresponding to that level
symbol_type leaf_to_symbol(leaf_config *leaf_point, level_type level, int dimensions, level_type max_depth)
{
    symbol_type result = 0;
    for (int j = 0; j < dimensions; j++)
    {
        int coordinate = leaf_point->coordinates[j];
        int bit = (coordinate >> (max_depth - level - 1)) & 1;
        result *= 2;
        result += bit;
    }
    return result;
}

// Used for Test script to check whether a leaf_point is present
bool md_trie::check(leaf_config *leaf_point, level_type strlen)
{
    level_type level = 0;
    if (root_ == NULL)
    {
        root_ = create_new_trie_node();
    }
    trie_node *current_trie_node = root_;
    treeblock *current_treeblock = walk_trie(current_trie_node, leaf_point, level);
    return walk_treeblock(current_treeblock, leaf_point, strlen, level);
}