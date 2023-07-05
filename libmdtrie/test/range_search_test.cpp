#include "catch.hpp"
#include "trie.h"

const int DIMENSION = 10;

bool
test_range_search(n_leaves_t n_points,
                  level_t max_depth,
                  level_t trie_depth,
                  preorder_t max_tree_nodes,
                  uint32_t n_itr)
{

  /**
      First insert random points into the mdtrie
      Range query random search range
      and check if number of found points is correct
  */

  bitmap::CompactPtrVector tmp_ptr_vect(n_points);
  p_key_to_treeblock_compact = &tmp_ptr_vect;
  create_level_to_num_children(std::vector<level_t>(DIMENSION, max_depth),
                               std::vector<level_t>(DIMENSION, 0),
                               max_depth);

  auto range = (point_t)pow(2, max_depth);
  auto* mdtrie = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_nodes);
  data_point<DIMENSION> leaf_point;
  std::vector<data_point<DIMENSION>> all_leaf_points;
  point_t max[DIMENSION];
  point_t min[DIMENSION];

  for (n_leaves_t itr = 1; itr <= n_points; itr++) {

    for (dimension_t i = 0; i < DIMENSION; i++) {
      leaf_point.set_coordinate(i, rand() % range);
      if (itr == 1) {
        max[i] = leaf_point.get_coordinate(i);
        min[i] = leaf_point.get_coordinate(i);
      } else {
        if (leaf_point.get_coordinate(i) > max[i]) {
          max[i] = leaf_point.get_coordinate(i);
        }
        if (leaf_point.get_coordinate(i) < min[i]) {
          min[i] = leaf_point.get_coordinate(i);
        }
      }
    }
    mdtrie->insert_trie(&leaf_point, itr - 1, p_key_to_treeblock_compact);
    all_leaf_points.push_back(leaf_point);
  }
  data_point<DIMENSION> start_range;
  data_point<DIMENSION> end_range;

  for (uint32_t itr = 1; itr <= n_itr; itr++) {

    std::vector<int32_t> found_points;

    for (dimension_t i = 0; i < DIMENSION; i++) {
      start_range.set_coordinate(i, min[i] + rand() % (max[i] - min[i] + 1));
      end_range.set_coordinate(
        i,
        start_range.get_coordinate(i) +
          rand() % (max[i] - start_range.get_coordinate(i) + 1));
    }

    mdtrie->range_search_trie(
      &start_range, &end_range, mdtrie->root(), 0, found_points);

    for (n_leaves_t i = 0; i < found_points.size() / DIMENSION; i++) {
      data_point<DIMENSION> leaf;
      for (dimension_t j = 0; j < DIMENSION; j++)
        leaf.set_coordinate(j, found_points[i * DIMENSION + j]);

      if (!mdtrie->check(&leaf)) {
        return false;
      }
    }

    n_leaves_t correct_found_size = 0;
    for (n_leaves_t i = 0; i < all_leaf_points.size(); i++) {

      bool in_range = true;
      data_point<DIMENSION> current_point = all_leaf_points[i];
      for (dimension_t j = 0; j < DIMENSION; j++) {
        if (current_point.get_coordinate(j) < start_range.get_coordinate(j) ||
            current_point.get_coordinate(j) > end_range.get_coordinate(j)) {
          in_range = false;
          break;
        }
      }
      if (in_range)
        correct_found_size++;
    }
    if (correct_found_size != found_points.size() / DIMENSION)
      return false;
  }

  delete mdtrie;
  return true;
}

TEST_CASE("Test Range Search", "[trie]")
{
  REQUIRE(test_range_search(100000, 6, 3, 128, 50000));
}
