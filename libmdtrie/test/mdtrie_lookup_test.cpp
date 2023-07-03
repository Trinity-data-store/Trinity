#include "catch.hpp"
#include "trie.h"

const int DIMENSION = 6;
const int DIMENSION_SMALL = 4;

bool test_lookup(n_leaves_t n_points, level_t max_depth, level_t trie_depth,
                 preorder_t max_tree_node) {

  /**
      First insert random points into the mdtrie
      Lookup query every points in the mdtrie given primary keys
  */

  bitmap::CompactPtrVector tmp_ptr_vect(n_points);
  p_key_to_treeblock_compact = &tmp_ptr_vect;
  create_level_to_num_children(std::vector<level_t>(DIMENSION, max_depth),
                               std::vector<level_t>(DIMENSION, 0), max_depth);

  auto range = (point_t)pow(2, max_depth);
  auto *mdtrie = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
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

  for (n_leaves_t i = 0; i < n_points; i++) {

    std::vector<morton_t> node_path_from_primary(max_depth + 1);
    tree_block<DIMENSION> *t_ptr =
        (tree_block<DIMENSION> *)(p_key_to_treeblock_compact->At(i));

    point_t parent_symbol_from_primary =
        t_ptr->get_node_path_primary_key(i, node_path_from_primary);
    node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

    auto returned_coordinates =
        t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);

    for (dimension_t j = 0; j < DIMENSION; j++) {
      if (returned_coordinates->get_coordinate(j) !=
          all_leaf_points[i].get_coordinate(j)) {
        return false;
      }
    }
  }
  delete mdtrie;
  return true;
}

bool test_lookup_close(n_leaves_t n_points, level_t max_depth,
                       level_t trie_depth, preorder_t max_tree_node) {

  /**
      First insert random points into the mdtrie
      Each attribute can only be from [0, 1023]
      Lookup query every points in the mdtrie given primary keys
  */

  bitmap::CompactPtrVector tmp_ptr_vect(n_points);
  p_key_to_treeblock_compact = &tmp_ptr_vect;
  create_level_to_num_children(std::vector<level_t>(DIMENSION, max_depth),
                               std::vector<level_t>(DIMENSION, 0), max_depth);

  auto range = (point_t)1024; // A close range
  auto *mdtrie = new md_trie<DIMENSION>(max_depth, trie_depth, max_tree_node);
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

  for (n_leaves_t i = 0; i < n_points; i++) {

    std::vector<morton_t> node_path_from_primary(max_depth + 1);
    tree_block<DIMENSION> *t_ptr =
        (tree_block<DIMENSION> *)(p_key_to_treeblock_compact->At(i));

    point_t parent_symbol_from_primary =
        t_ptr->get_node_path_primary_key(i, node_path_from_primary);
    node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

    auto returned_coordinates =
        t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);

    for (dimension_t j = 0; j < DIMENSION; j++) {
      if (returned_coordinates->get_coordinate(j) !=
          all_leaf_points[i].get_coordinate(j)) {
        return false;
      }
    }
  }
  delete mdtrie;
  return true;
}

bool test_lookup_small_dimension(n_leaves_t n_points, level_t max_depth,
                                 level_t trie_depth, preorder_t max_tree_node) {

  /**
      First insert random points into the mdtrie
      Each attribute can only be from [0, 1023]
      Lookup query every points in the mdtrie given primary keys
      Test case: small number of dimensions
  */

  bitmap::CompactPtrVector tmp_ptr_vect(n_points);
  p_key_to_treeblock_compact = &tmp_ptr_vect;
  create_level_to_num_children(std::vector<level_t>(DIMENSION_SMALL, max_depth),
                               std::vector<level_t>(DIMENSION_SMALL, 0),
                               max_depth);

  auto range = (point_t)1024; // A close range
  auto *mdtrie =
      new md_trie<DIMENSION_SMALL>(max_depth, trie_depth, max_tree_node);
  data_point<DIMENSION_SMALL> leaf_point;
  std::vector<data_point<DIMENSION_SMALL>> all_leaf_points;
  point_t max[DIMENSION_SMALL];
  point_t min[DIMENSION_SMALL];

  for (n_leaves_t itr = 1; itr <= n_points; itr++) {

    for (dimension_t i = 0; i < DIMENSION_SMALL; i++) {
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

  for (n_leaves_t i = 0; i < n_points; i++) {

    std::vector<morton_t> node_path_from_primary(max_depth + 1);
    tree_block<DIMENSION_SMALL> *t_ptr =
        (tree_block<DIMENSION_SMALL> *)(p_key_to_treeblock_compact->At(i));

    point_t parent_symbol_from_primary =
        t_ptr->get_node_path_primary_key(i, node_path_from_primary);
    node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

    auto returned_coordinates = t_ptr->node_path_to_coordinates(
        node_path_from_primary, DIMENSION_SMALL);

    for (dimension_t j = 0; j < DIMENSION_SMALL; j++) {
      if (returned_coordinates->get_coordinate(j) !=
          all_leaf_points[i].get_coordinate(j)) {
        return false;
      }
    }
  }
  delete mdtrie;
  return true;
}

TEST_CASE("Check Lookup", "[trie]") {
  REQUIRE(test_lookup(100000, 10, 3, 128));
}

TEST_CASE("Check Lookup Close", "[trie]") {
  REQUIRE(test_lookup_close(100000, 10, 3, 128));
}

TEST_CASE("Check Lookup Small Dimensions", "[trie]") {
  REQUIRE(test_lookup_small_dimension(100000, 10, 3, 128));
}