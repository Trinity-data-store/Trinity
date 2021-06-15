#ifndef MD_TRIE_DATA_POINT_H
#define MD_TRIE_DATA_POINT_H

#include "defs.h"

// Struct for each point that we want to insert
class data_point {
public:
    explicit data_point(dimension_t dimensions) : coordinates(dimensions) {}

    symbol_t leaf_to_symbol(level_t level, level_t max_depth);

    void get_representation(data_point *end_range, uint8_t *representation, level_t level, level_t max_depth);

    void update_range(data_point *end_range, const uint8_t *representation, level_t level, level_t max_depth);

    coordinates_t coordinates; // TODO(anuragk): Should be private...
};

#endif //MD_TRIE_DATA_POINT_H
