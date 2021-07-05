#ifndef MD_TRIE_DATA_POINT_H
#define MD_TRIE_DATA_POINT_H

#include "defs.h"
#include <cstdlib>
#include <cstring>
#include "bitmap.h"

// Struct for each point that we want to insert
template<dimension_t DIMENSION>
class data_point {
private:
    point_t coordinates[DIMENSION] = {};
public:
    explicit data_point(){
        
    };

    inline void set(coordinates_t new_coordinates){
        memcpy(coordinates, new_coordinates, sizeof(point_t) * DIMENSION);
    }

    inline coordinates_t get(){
        return coordinates;
    }

    inline point_t get_coordinate(dimension_t index){
        return coordinates[index];
    }

    inline void set_coordinate(dimension_t index, point_t value){
        coordinates[index] = value;
    }

    // Given the leaf_point and the level we are at, return the Morton code corresponding to that level
    inline symbol_t leaf_to_symbol(level_t level, level_t max_depth) {

        symbol_t result = 0;
        level_t offset = max_depth - level - 1U; 
        // size_t coordinate_size = coordinates.size();
        for (size_t i = 0; i < DIMENSION; i++) {
            point_t coordinate = coordinates[i];
            bool bit = GETBIT(coordinate, offset); 
            result = (result << 1U) + bit;
        }
        return result;
    }

    inline bool update_range_morton(data_point<DIMENSION> *end_range, symbol_t current_morton, level_t level, level_t max_depth) {

        level_t offset = max_depth - level - 1U;    
        for (size_t j = 0; j < DIMENSION; j++) {
            point_t start_coordinate = coordinates[j];
            point_t end_coordinate = end_range->coordinates[j];
            dimension_t morton_offset = DIMENSION - j - 1U;

            bool start_bit = GETBIT(start_coordinate, offset); 
            bool end_bit = GETBIT(end_coordinate, offset);
            bool morton_bit = GETBIT(current_morton, morton_offset);
            
            // Bring the start of the search range to second half
            if (morton_bit && !start_bit) {
                start_coordinate = start_coordinate & bitmap::low_bits_unset[offset];
                SETBIT(start_coordinate, offset);
            } 
            // Bring the end of the search range to first half
            else if (!morton_bit && end_bit) {
                end_coordinate = end_coordinate | bitmap::low_bits_set[offset];
                // In the end, only the start_coordinate is kept as the returned point
                // CLRBIT(end_coordinate, (offset + 1));            
            }
            coordinates[j] = start_coordinate;
            end_range->coordinates[j] = end_coordinate;
        }
        return true;
    }
};

#endif //MD_TRIE_DATA_POINT_H
