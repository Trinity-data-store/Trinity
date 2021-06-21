#include "data_point.h"
#include "bitmap.h"

// Obtain representation, where 2 stands for "?"
// For example, 0?1 means it can be either 001 or 011

void
data_point::get_representation(data_point *end_range, representation_t *representation, level_t level, level_t max_depth) {
    // size_t coordinate_size = coordinates.size();
    // 0010 - A
    // 0110 - B
    // 0210 - representation

    // 0010 - A & B
    // 0100 - A ^ B

    level_t offset = max_depth - level - 1U; 
    for (size_t j = 0; j < size_; j++) {
        point_t start_coordinate = coordinates[j];
        point_t end_coordinate = end_range->coordinates[j];
        bool start_bit = GETBIT(start_coordinate, offset); 
        bool end_bit = GETBIT(end_coordinate, offset);
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
    level_t offset = max_depth - level - 1U; 
    // size_t coordinate_size = coordinates.size();
    for (size_t i = 0; i < size_; i++) {
        point_t coordinate = coordinates[i];
        bool bit =  GETBIT(coordinate, offset); 
        result = (result << 1U) + bit;
    }
    return result;
}


bool data_point::update_range(data_point *end_range, const representation_t *representation, level_t level, level_t max_depth) {
    // size_t coordinate_size = coordinates.size();
    level_t offset = max_depth - level - 1U;    
    for (size_t j = 0; j < size_; j++) {
        point_t start_coordinate = coordinates[j];
        point_t end_coordinate = end_range->coordinates[j];
        // Change to GETBIT in bitmap.h
        bool start_bit = GETBIT(start_coordinate, offset); 
        bool end_bit = GETBIT(end_coordinate, offset);
        if ((representation[j] && !end_bit) || (!representation[j] && start_bit)){
            return false;
        }
        if (representation[j] && !start_bit) {
            start_coordinate = start_coordinate & bitmap::low_bits_unset[offset];
            SETBIT(start_coordinate, offset);
        } 
        else if (!representation[j] && end_bit) {
            end_coordinate = end_coordinate & bitmap::low_bits_unset[offset + 1];
            end_coordinate = end_coordinate | bitmap::low_bits_set[offset];
        }
        coordinates[j] = start_coordinate;
        end_range->coordinates[j] = end_coordinate;
    }
    return true;
}

