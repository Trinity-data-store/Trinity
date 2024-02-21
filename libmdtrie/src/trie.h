#ifndef MD_TRIE_MD_TRIE_H
#define MD_TRIE_MD_TRIE_H

#include "compressed_bitmap.h"
#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <queue>
#include <sys/stat.h>
#include <utility>
#include <vector>
#include <algorithm>

#include "data_point.h"
#include "defs.h"
#include "point_array.h"
#include "tree_block.h"
#include "trie_node.h"
#include <unordered_set>

template <dimension_t DIMENSION>
class md_trie
{
public:
  explicit md_trie(level_t max_depth,
                   level_t trie_depth,
                   n_leaves_t max_tree_nodes,
                   std::vector<level_t> bit_widths,
                   std::vector<level_t> start_bits)
  {
    _max_depth = max_depth;
    trie_depth_ = trie_depth;
    max_tree_nodes_ = max_tree_nodes;
    _create_level_to_num_children(bit_widths, start_bits, max_depth);
    root_ = new trie_node<DIMENSION>(false, level_to_num_children_[0]);
    ptr_vector_ = new bitmap::CompactPtrVector(total_points_count);
  }

  tree_block<DIMENSION> *_walk_trie(trie_node<DIMENSION> *current_trie_node,
                                    data_point<DIMENSION> *leaf_point,
                                    level_t &level) const
  {

    morton_t current_symbol;

    while (level < trie_depth_ &&
           current_trie_node->get_child(leaf_point->leaf_to_symbol(level, dimension_to_num_bits_, start_dimension_bits_)))
    {

      current_trie_node =
          current_trie_node->get_child(leaf_point->leaf_to_symbol(level, dimension_to_num_bits_, start_dimension_bits_));
      level++;
    }
    while (level < trie_depth_)
    {

      current_symbol = leaf_point->leaf_to_symbol(level, dimension_to_num_bits_, start_dimension_bits_);
      if (level == trie_depth_ - 1)
      {
        current_trie_node->set_child(
            current_symbol,
            new trie_node<DIMENSION>(true, level_to_num_children_[level + 1]));
      }
      else
      {
        current_trie_node->set_child(
            current_symbol,
            new trie_node<DIMENSION>(false, level_to_num_children_[level + 1]));
      }
      current_trie_node->get_child(current_symbol)
          ->set_parent_trie_node(current_trie_node);
      current_trie_node->get_child(current_symbol)
          ->set_parent_symbol(current_symbol);
      current_trie_node = current_trie_node->get_child(current_symbol);
      level++;
    }

    tree_block<DIMENSION> *current_treeblock = nullptr;
    if (current_trie_node->get_block() == nullptr)
    {
      current_treeblock =
          new tree_block<DIMENSION>(trie_depth_,
                                    1 /* initial_tree_capacity_ */,
                                    1 << level_to_num_children_[trie_depth_],
                                    1,
                                    _max_depth,
                                    max_tree_nodes_,
                                    current_trie_node,
                                    level_to_num_children_,
                                    dimension_to_num_bits_,
                                    start_dimension_bits_);
      current_trie_node->set_block(current_treeblock);
    }
    else
    {
      current_treeblock =
          (tree_block<DIMENSION> *)current_trie_node->get_block();
    }
    return current_treeblock;
  }

  void insert_trie(data_point<DIMENSION> *leaf_point,
                   n_leaves_t primary_key)
  {

    level_t level = 0;
    trie_node<DIMENSION> *current_trie_node = root_;
    tree_block<DIMENSION> *current_treeblock =
        _walk_trie(current_trie_node, leaf_point, level);
    current_treeblock->insert_remaining(
        leaf_point, level, primary_key, ptr_vector_);
  }

  data_point<DIMENSION> *lookup_trie(n_leaves_t primary_key)
  {
    std::vector<morton_t> node_path_from_primary(_max_depth + 1);
    tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *)ptr_vector_->At(primary_key);
    morton_t parent_symbol_from_primary =
        t_ptr->get_node_path_primary_key(primary_key, node_path_from_primary);
    node_path_from_primary[_max_depth - 1] = parent_symbol_from_primary;
    return t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);
  }

  std::vector<int32_t> lookup_trie_vect(n_leaves_t primary_key)
  {
    std::vector<morton_t> node_path_from_primary(_max_depth + 1);
    tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *)ptr_vector_->At(primary_key);
    morton_t parent_symbol_from_primary =
        t_ptr->get_node_path_primary_key(primary_key, node_path_from_primary);
    node_path_from_primary[_max_depth - 1] = parent_symbol_from_primary;
    return t_ptr->node_path_to_coordinates_vect(node_path_from_primary, DIMENSION);
  }

  bool check(data_point<DIMENSION> *leaf_point) const
  {

    level_t level = 0;
    trie_node<DIMENSION> *current_trie_node = root_;
    tree_block<DIMENSION> *current_treeblock =
        _walk_trie(current_trie_node, leaf_point, level);
    bool result = current_treeblock->walk_tree_block(leaf_point, level);
    return result;
  }

  uint64_t size()
  {

    uint64_t total_size = sizeof(root_) + sizeof(_max_depth);
    total_size += sizeof(max_tree_nodes_);

    std::queue<trie_node<DIMENSION> *> trie_node_queue;
    trie_node_queue.push(root_);
    level_t current_level = 0;

    while (!trie_node_queue.empty())
    {

      unsigned int queue_size = trie_node_queue.size();

      for (unsigned int s = 0; s < queue_size; s++)
      {

        trie_node<DIMENSION> *current_node = trie_node_queue.front();
        trie_node_queue.pop();

        total_size +=
            current_node->size(1 << level_to_num_children_[current_level],
                               current_level == trie_depth_);

        if (current_level != trie_depth_)
        {
          for (int i = 0; i < (1 << level_to_num_children_[current_level]);
               i++)
          {
            if (current_node->get_child(i))
            {
              trie_node_queue.push(current_node->get_child(i));
            }
          }
        }
        else
        {
          total_size += current_node->get_block()->size();
        }
      }
      current_level++;
    }
    // Global variables in def.h
    total_size += sizeof(total_points_count);
    total_size += sizeof(level_to_num_children_);
    total_size += sizeof(max_tree_nodes_);
    total_size += sizeof(_max_depth);
    total_size += sizeof(trie_depth_);

    total_size += sizeof(ptr_vector_);
    total_size += ptr_vector_->size_overhead();

    total_size += sizeof(dimension_to_num_bits_);
    total_size += sizeof(start_dimension_bits_);

    return total_size;
  }

  void range_search_trie(data_point<DIMENSION> *start_range,
                         data_point<DIMENSION> *end_range,
                         std::vector<int32_t> &found_points)
  {
    _range_search_trie(start_range, end_range, root_, 0, found_points);
  }

  void _range_search_trie(data_point<DIMENSION> *start_range,
                          data_point<DIMENSION> *end_range,
                          trie_node<DIMENSION> *current_trie_node,
                          level_t level,
                          std::vector<int32_t> &found_points)
  {
    if (level == trie_depth_)
    {

      auto *current_treeblock =
          (tree_block<DIMENSION> *)current_trie_node->get_block();
      current_treeblock->range_search_treeblock(start_range,
                                                end_range,
                                                current_treeblock,
                                                level,
                                                0,
                                                0,
                                                0,
                                                0,
                                                0,
                                                0,
                                                found_points);
      return;
    }

    morton_t start_symbol = start_range->leaf_to_symbol(level, dimension_to_num_bits_, start_dimension_bits_);
    morton_t end_symbol = end_range->leaf_to_symbol(level, dimension_to_num_bits_, start_dimension_bits_);
    morton_t representation = start_symbol ^ end_symbol;
    morton_t neg_representation = ~representation;

    struct data_point<DIMENSION> original_start_range = (*start_range);
    struct data_point<DIMENSION> original_end_range = (*end_range);
    for (morton_t current_symbol = start_symbol; current_symbol <= end_symbol;
         current_symbol++)
    {

      if ((start_symbol & neg_representation) !=
          (current_symbol & neg_representation))
      {
        continue;
      }

      if (!current_trie_node->get_child(current_symbol))
      {
        continue;
      }

      start_range->update_symbol(end_range, current_symbol, level, level_to_num_children_[level], dimension_to_num_bits_, start_dimension_bits_);

      _range_search_trie(start_range,
                         end_range,
                         current_trie_node->get_child(current_symbol),
                         level + 1,
                         found_points);
      (*start_range) = original_start_range;
      (*end_range) = original_end_range;
    }
  }

  void _create_level_to_num_children(std::vector<level_t> bit_widths,
                                     std::vector<level_t> start_bits,
                                     level_t max_level)
  {
    dimension_to_num_bits_ = std::vector<level_t>(bit_widths);
    start_dimension_bits_ = std::vector<level_t>(start_bits);
    dimension_t num_dimensions = bit_widths.size();

    for (level_t level = 0; level < max_level; level++)
    {

      dimension_t dimension_left = num_dimensions;
      for (dimension_t j = 0; j < num_dimensions; j++)
      {

        if (level + 1 > bit_widths[j] || level < start_dimension_bits_[j])
          dimension_left--;
      }
      level_to_num_children_.push_back(dimension_left);
    }
  }

private:
  trie_node<DIMENSION> *root_ = nullptr;
  level_t _max_depth;
  n_leaves_t max_tree_nodes_;
  bitmap::CompactPtrVector *ptr_vector_ = nullptr;
  std::vector<morton_t> level_to_num_children_;
  std::vector<level_t> dimension_to_num_bits_;
  std::vector<level_t> start_dimension_bits_;
};

const dimension_t _BASE_DIMENSION = 8;

template <dimension_t DIMENSION>
class MdTries
{
public:
  MdTries(level_t max_depth,
          level_t trie_depth,
          n_leaves_t max_tree_nodes,
          std::vector<level_t> bit_widths,
          std::vector<level_t> start_bits)
  {
    // TODO(MaoZiming): error when DIMENSION is not a multiple of 10.
    for (dimension_t i = 0; i < DIMENSION; i += _BASE_DIMENSION)
    {
      auto new_bit_widths = std::vector<level_t>(bit_widths.begin() + i,
                                                 bit_widths.begin() + i + _BASE_DIMENSION);

      auto new_start_bits = std::vector<level_t>(start_bits.begin() + i,
                                                 start_bits.begin() + i + _BASE_DIMENSION);
      _mdtries.push_back(new md_trie<_BASE_DIMENSION>(max_depth,
                                                      trie_depth,
                                                      max_tree_nodes, new_bit_widths, new_start_bits));
    }
    _max_depth = max_depth;
  }

  void insert_trie(data_point<DIMENSION> *point, int p_key)
  {
    for (dimension_t i = 0; i < DIMENSION; i += _BASE_DIMENSION)
    {
      data_point<_BASE_DIMENSION> p;
      for (dimension_t j = i; j < i + _BASE_DIMENSION; j++)
      {
        p.set_coordinate(j - i, point->get_coordinate(j));
      }
      _mdtries[i / _BASE_DIMENSION]->insert_trie(&p, p_key);
    }
  }

  uint64_t size(void)
  {
    uint64_t total_size = 0;
    for (size_t i = 0; i < _mdtries.size(); i++)
    {
      total_size += _mdtries[i]->size();
    }
    return total_size;
  }

  data_point<DIMENSION> *lookup_trie(int p_key)
  {
    auto return_point = new data_point<DIMENSION>();
    for (dimension_t i = 0; i < DIMENSION; i += _BASE_DIMENSION)
    {
      data_point<_BASE_DIMENSION> *pt = _mdtries[i / _BASE_DIMENSION]->lookup_trie(p_key);
      for (dimension_t j = 0; j < _BASE_DIMENSION; j++)
      {
        return_point->set_coordinate(i, pt->get_coordinate(j));
      }
    }
    return return_point;
  }

  // Function to find the intersection of two vectors
  std::vector<int> _intersection(const std::vector<int> &v1, const std::vector<int> &v2)
  {
    // Create unordered sets from the vectors for faster lookup
    std::unordered_set<int> set1(v1.begin(), v1.end());
    std::unordered_set<int> set2(v2.begin(), v2.end());

    // Initialize a vector to store the intersection
    std::vector<int> intersect;

    // Iterate through set1 and check if each element is present in set2
    for (int num : set1)
    {
      if (set2.find(num) != set2.end())
      {
        intersect.push_back(num);
      }
    }

    return intersect;
  }

  void range_search_trie(data_point<DIMENSION> *from_range, data_point<DIMENSION> *to_range, std::vector<int32_t> &found_points)
  {
    for (dimension_t i = 0; i < DIMENSION; i += _BASE_DIMENSION)
    {
      data_point<_BASE_DIMENSION> start;
      data_point<_BASE_DIMENSION> to;
      std::vector<int> vect;
      for (dimension_t j = i; j < i + _BASE_DIMENSION; j++)
      {
        start.set_coordinate(j - i, from_range->get_coordinate(j));
        to.set_coordinate(j - i, to_range->get_coordinate(j));
      }

      _mdtries[i / _BASE_DIMENSION]->range_search_trie(
          &start, &to, vect);

      if (vect.size() == 0)
        return;

      if (i == 0)
        found_points = std::vector<int>(vect);
      else
        found_points = _intersection(found_points, vect);
    }
  }

protected:
  std::vector<md_trie<_BASE_DIMENSION> *> _mdtries;
  level_t _max_depth;
};

#endif // MD_TRIE_MD_TRIE_H
