#include "data_point.h"

// Obtain representation, where 2 stands for "?"
// For example, 0?1 means it can be either 001 or 011
void
data_point::get_representation(data_point *end_range, uint8_t *representation, level_t level, level_t max_depth) {
    // size_t coordinate_size = coordinates.size();  
    for (size_t j = 0; j < size_; j++) {
        point_t start_coordinate = coordinates[j];
        point_t end_coordinate = end_range->coordinates[j];
        bool start_bit = (start_coordinate >> (max_depth - level - 1U)) & 1U;
        bool end_bit = (end_coordinate >> (max_depth - level - 1U)) & 1U;
        if (start_bit == end_bit) {
            representation[j] = start_bit;
        } else {
            representation[j] = 2;
        }
    }
}

// Given the leaf_point and the level we are at, return the Morton code corresponding to that level
symbol_t data_point::leaf_to_symbol(level_t level, level_t max_depth) {
    symbol_t result = 0;
    // size_t coordinate_size = coordinates.size();
    for (size_t i = 0; i < size_; i++) {
        point_t coordinate = coordinates[i];
        bool bit = (coordinate >> (max_depth - level - 1U)) & 1U;
        result = (result << 1U) + bit;
    }
    return result;
}


void data_point::update_range(data_point *end_range, const uint8_t *representation, level_t level, level_t max_depth) {
    // size_t coordinate_size = coordinates.size();    
    for (size_t j = 0; j < size_; j++) {
        point_t start_coordinate = coordinates[j];
        point_t end_coordinate = end_range->coordinates[j];
        bool start_bit = (start_coordinate >> (max_depth - level - 1U)) & 1U;
        bool end_bit = (end_coordinate >> (max_depth - level - 1U)) & 1U;
        if (representation[j] == 1) {
            if (start_bit == 0) {
                start_coordinate =
                        ((start_coordinate >> (max_depth - level - 1U)) | 1U) << (max_depth - level - 1U);
            }
            if (end_bit == 0) {
                return;
            }
        } else if (representation[j] == 0) {
            if (start_bit == 1) {
                return;
            }
            if (end_bit == 1) {
                end_coordinate = ((end_coordinate >> (max_depth - level)) << (max_depth - level)) +
                                 ((1U << (max_depth - level - 1U)) - 1U);
            }
        }
        coordinates[j] = start_coordinate;
        end_range->coordinates[j] = end_coordinate;
    }
}

