#ifndef MD_TRIE_MD_TRIE_H
#define MD_TRIE_MD_TRIE_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <utility>
#include <cstdint>
#include <ctime>
#include <csignal>
#include "compressed_bitmap.h"
#include <sys/stat.h>
#include <vector>
#include <queue>

#include "defs.h"
#include "data_point.h"
#include "point_array.h"
#include "tree_block.h"
#include "trie_node.h"

template<dimension_t DIMENSION>
class md_trie {
public:
    explicit md_trie(level_t max_depth, level_t trie_depth,
                     preorder_t max_tree_nodes){ 
    
        max_depth_ = max_depth;
        trie_depth_ = trie_depth;
        max_tree_nodes_ = max_tree_nodes;
        root_ = new trie_node<DIMENSION>(false, level_to_num_children[0]);
    }

    inline trie_node<DIMENSION> *root() {

        return root_;
    }

    tree_block<DIMENSION> *walk_trie(trie_node<DIMENSION> *current_trie_node, data_point<DIMENSION> *leaf_point, level_t &level) const {

        morton_t current_symbol;

        while (level < trie_depth_ && current_trie_node->get_child(leaf_point->leaf_to_symbol(level))){
            
            current_trie_node = current_trie_node->get_child(leaf_point->leaf_to_symbol(level));
            level++;
        }
        while (level < trie_depth_) {

            current_symbol = leaf_point->leaf_to_symbol(level);
            if (level == trie_depth_ - 1){
                current_trie_node->set_child(current_symbol, new trie_node<DIMENSION>(true, level_to_num_children[level + 1]));
            }
            else {
                current_trie_node->set_child(current_symbol, new trie_node<DIMENSION>(false, level_to_num_children[level + 1]));
            }
            current_trie_node->get_child(current_symbol)->set_parent_trie_node(current_trie_node);
            current_trie_node->get_child(current_symbol)->set_parent_symbol(current_symbol);
            current_trie_node = current_trie_node->get_child(current_symbol);
            level++;
        }

        tree_block<DIMENSION> *current_treeblock = nullptr;
        if (current_trie_node->get_block() == nullptr) {
            current_treeblock = new tree_block<DIMENSION>(trie_depth_, 1 /* initial_tree_capacity_ */, 1 << level_to_num_children[trie_depth_], 1, max_depth_, max_tree_nodes_, current_trie_node);
            current_trie_node->set_block(current_treeblock);
        } 
        else {
            current_treeblock = (tree_block<DIMENSION> *) current_trie_node->get_block();
        }
        return current_treeblock;
    }

    void insert_trie(data_point<DIMENSION> *leaf_point, n_leaves_t primary_key, bitmap::CompactPtrVector *p_key_to_treeblock_compact) {

        level_t level = 0;
        trie_node<DIMENSION> *current_trie_node = root_;
        tree_block<DIMENSION> *current_treeblock = walk_trie(current_trie_node, leaf_point, level);
        current_treeblock->insert_remaining(leaf_point, level, primary_key, p_key_to_treeblock_compact);
    }

    bool check(data_point<DIMENSION> *leaf_point) const {

        level_t level = 0;
        trie_node<DIMENSION> *current_trie_node = root_;
        tree_block<DIMENSION> *current_treeblock = walk_trie(current_trie_node, leaf_point, level);
        bool result = current_treeblock->walk_tree_block(leaf_point, level);
        return result;
    }

    uint64_t size(bitmap::CompactPtrVector *p_key_to_treeblock_compact) {

        uint64_t total_size = sizeof(root_) + sizeof(max_depth_);
        total_size += sizeof(max_tree_nodes_); 

        std::queue<trie_node<DIMENSION> *> trie_node_queue;
        trie_node_queue.push(root_);
        level_t current_level = 0;

        while (!trie_node_queue.empty()){

            unsigned int queue_size = trie_node_queue.size();

            for (unsigned int s = 0; s < queue_size; s++){

                trie_node<DIMENSION> *current_node = trie_node_queue.front();
                trie_node_queue.pop(); 

                total_size += current_node->size(1 << level_to_num_children[current_level], current_level == trie_depth_);
                
                if (current_level != trie_depth_) {
                    for (int i = 0; i < (1 << level_to_num_children[current_level]); i++)
                    {
                        if (current_node->get_child(i)) {
                            trie_node_queue.push(current_node->get_child(i));
                        }
                    }
                }
                else {
                    total_size += current_node->get_block()->size();
                }
            }
            current_level ++;
        }
        // Global variables in def.h
        total_size += sizeof(total_points_count);
        total_size += sizeof(discount_factor);
        total_size += sizeof(level_to_num_children) + 32 * sizeof(morton_t);
        total_size += sizeof(max_tree_nodes_);
        total_size += sizeof(max_depth_);
        total_size += sizeof(trie_depth_);

        total_size += sizeof(p_key_to_treeblock_compact);
        total_size += p_key_to_treeblock_compact->size_overhead();

        total_size += sizeof(dimension_to_num_bits) + dimension_to_num_bits.size() * sizeof(morton_t);
        total_size += sizeof(start_dimension_bits) + start_dimension_bits.size() * sizeof(level_t);
        total_size += sizeof(no_dynamic_sizing);

        return total_size;
    }

    void range_search_trie(data_point<DIMENSION> *start_range, data_point<DIMENSION> *end_range, trie_node<DIMENSION> *current_trie_node,
                                    level_t level, std::vector<int32_t> &found_points) {

        if (level == trie_depth_) {

            auto *current_treeblock = (tree_block<DIMENSION> *) current_trie_node->get_block();
            
            current_treeblock->range_search_treeblock(start_range, end_range, current_treeblock, level, 0, 0, 0, 0, 0, 0, found_points);
            return;
        }
        
        morton_t start_symbol = start_range->leaf_to_symbol(level);
        morton_t end_symbol = end_range->leaf_to_symbol(level);
        morton_t representation = start_symbol ^ end_symbol;
        morton_t neg_representation = ~representation;

        struct data_point<DIMENSION> original_start_range = (*start_range);
        struct data_point<DIMENSION> original_end_range = (*end_range); 

        for (morton_t current_symbol = start_symbol; current_symbol <= end_symbol; current_symbol++){

            if ((start_symbol & neg_representation) != (current_symbol & neg_representation)){
                continue;
            }            
            
            if (!current_trie_node->get_child(current_symbol)) {
                continue;
            }

            start_range->update_symbol(end_range, current_symbol, level);

            range_search_trie(start_range, end_range, current_trie_node->get_child(current_symbol), level + 1,
                                found_points);

            (*start_range) = original_start_range;
            (*end_range) = original_end_range;                
        }
    }


    size_t Serialize(std::ostream& out) {

        // raise(SIGINT);
        
        size_t out_size = 0;

        out_size += p_key_to_treeblock_compact->Serialize(out);         

        // trie_node *root_ = nullptr;
        out.write(reinterpret_cast<const char *>(&root_), sizeof(root_));
        out_size += sizeof(root_);     

        // level_t max_depth_;
        out.write(reinterpret_cast<const char *>(&max_depth_), sizeof(max_depth_));
        out_size += sizeof(max_depth_);      

        // preorder_t max_tree_nodes_;
        out.write(reinterpret_cast<const char *>(&max_tree_nodes_), sizeof(max_tree_nodes_));
        out_size += sizeof(max_tree_nodes_);         

        out.write(reinterpret_cast<char const*>(&total_points_count), sizeof(total_points_count));
        out_size += sizeof(total_points_count);     


        std::queue<trie_node<DIMENSION> *> trie_node_queue;
        trie_node_queue.push(root_);
        level_t current_level = 0;

        while (!trie_node_queue.empty()){

            // trie_node<DIMENSION> *current_node = trie_node_queue.front();
            // trie_node_queue.pop();

            // out_size += current_node->Serialize(out);
            unsigned int queue_size = trie_node_queue.size();

            for (unsigned int s = 0; s < queue_size; s++){

                trie_node<DIMENSION> *current_node = trie_node_queue.front();
                trie_node_queue.pop(); 
                out_size += current_node->Serialize(out, 1 << level_to_num_children[current_level], current_level == trie_depth_);
                
                if (current_level != trie_depth_) {
                    for (int i = 0; i < (1 << level_to_num_children[current_level]); i++)
                    {
                        if (current_node->get_child(i)) {
                            trie_node_queue.push(current_node->get_child(i));
                        }
                    }
                }
                else {
                    out_size += current_node->get_block()->Serialize(out);
                }
            }
            current_level ++;                
        }

        // out.write(reinterpret_cast<char const*>(&total_points_count), sizeof(total_points_count));
        // out_size += sizeof(total_points_count);     

        // out_size += p_key_to_treeblock_compact->Serialize(out);         

        // total_points_count = 135;
        // out.write(reinterpret_cast<char const*>(&total_points_count), sizeof(total_points_count));
        // out_size += sizeof(total_points_count);   

        return out_size;
    }


    size_t Deserialize(std::istream& in) {

        // raise(SIGINT);

        size_t in_size = 0;
        // raise(SIGINT);

        p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);
        in_size += p_key_to_treeblock_compact->Deserialize(in);

        in.read(reinterpret_cast<char *>(&root_), sizeof(root_));
        in_size += sizeof(root_);     
        old_ptr_to_new_ptr[root_] = (void *) new trie_node<DIMENSION>(false, level_to_num_children[0]);
        root_ = (trie_node<DIMENSION> *) old_ptr_to_new_ptr[root_];

        in.read(reinterpret_cast<char *>(&max_depth_), sizeof(max_depth_));
        in_size += sizeof(max_depth_);      

        in.read(reinterpret_cast<char *>(&max_tree_nodes_), sizeof(max_tree_nodes_));
        in_size += sizeof(max_tree_nodes_);         

        n_leaves_t tmp = 0;
        in.read(reinterpret_cast<char *>(&tmp), sizeof(total_points_count));
        in_size += sizeof(total_points_count); 
        total_points_count = tmp;


        // bitmap::CompactPtrVector tmp_ptr_vect(total_points_count);
        // p_key_to_treeblock_compact = &tmp_ptr_vect;

        std::queue<trie_node<DIMENSION> *> trie_node_queue;
        trie_node_queue.push(root_);
        level_t current_level = 0;

        while (!trie_node_queue.empty()){

            unsigned int queue_size = trie_node_queue.size();

            for (unsigned int s = 0; s < queue_size; s++){

                trie_node<DIMENSION> *current_node = trie_node_queue.front();

                trie_node_queue.pop(); 
                in_size += current_node->Deserialize(in, 1 << level_to_num_children[current_level], current_level == trie_depth_);

                if (current_level != trie_depth_) {
                    for (int i = 0; i < (1 << level_to_num_children[current_level]); i++)
                    {
                        if (current_node->get_child(i)) {
                            old_ptr_to_new_ptr[current_node->get_child(i)] = (void *) new trie_node<DIMENSION>(false, level_to_num_children[current_level]);
                            current_node->set_child(i, (trie_node<DIMENSION> *) old_ptr_to_new_ptr[current_node->get_child(i)]);
                            trie_node_queue.push(current_node->get_child(i));
                        }
                    }
                }
                else {
                    // if (current_node->get_block()->get_primary_key_list()[0].check_if_present(171328))
                    //     raise(SIGINT);

                    old_ptr_to_new_ptr[current_node->get_block()] = new tree_block<DIMENSION>(trie_depth_, 1 /* initial_tree_capacity_ */, 1 << level_to_num_children[trie_depth_], 1, max_depth_, max_tree_nodes_, current_node);
                    current_node->set_block((tree_block<DIMENSION> *) old_ptr_to_new_ptr[current_node->get_block()]);
                    current_node->get_block()->Deserialize(in);
                }
            }
            current_level ++;   
        }

        // On stack
        // p_key_to_treeblock_compact = new bitmap::CompactPtrVector(total_points_count);
        // in_size += p_key_to_treeblock_compact->Deserialize(in);

        // n_leaves_t random_num = 0;
        // in.read(reinterpret_cast<char *>(&random_num), sizeof(total_points_count));
        // in_size += sizeof(total_points_count); 

        for (unsigned int i = 0; i < total_points_count; i++){

            p_key_to_treeblock_compact->Set(i, old_ptr_to_new_ptr[p_key_to_treeblock_compact->At(i)]);
        }
        return in_size;
    }
    
private:
    trie_node<DIMENSION> *root_ = nullptr;
    level_t max_depth_;
    preorder_t max_tree_nodes_;
};

#endif //MD_TRIE_MD_TRIE_H
