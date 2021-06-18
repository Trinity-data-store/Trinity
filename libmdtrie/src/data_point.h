#ifndef MD_TRIE_DATA_POINT_H
#define MD_TRIE_DATA_POINT_H

#include "defs.h"

// Struct for each point that we want to insert
class data_point {
private:
    coordinates_t coordinates;
    dimension_t size_;
public:
    explicit data_point(dimension_t dimensions) : coordinates(dimensions) {
        size_ = dimensions;
    }

    symbol_t leaf_to_symbol(level_t level, level_t max_depth);

    void get_representation(data_point *end_range, uint8_t *representation, level_t level, level_t max_depth);

    bool update_range(data_point *end_range, const uint8_t *representation, level_t level, level_t max_depth);

    void set(coordinates_t new_coordinates){
        coordinates = new_coordinates;
    }

    coordinates_t get(){
        return coordinates;
    }

    point_t get_coordinate(dimension_t index){
        return coordinates[index];
    }

    void set_coordinate(dimension_t index, point_t value){
        coordinates[index] = value;
    }

};

#endif //MD_TRIE_DATA_POINT_H
