#ifndef MD_TRIE_DATA_POINT_H
#define MD_TRIE_DATA_POINT_H

#include "defs.h"
#include <cstdlib>
#include <cstring>
#include "compressed_bitmap.h"
#include <bit>
#include <bitset>
#include <array>

class data_point {
public:
    
    explicit data_point(){}

    inline point_t *get(){
        return coordinates_;
    }

    inline void set(point_t *coordinates){

        memcpy(coordinates_, coordinates, sizeof(point_t) * DATA_DIMENSION);
    }

    inline point_t get_coordinate(dimension_t index){

        return coordinates_[index];
    }

    inline void set_coordinate(dimension_t index, point_t value){

        coordinates_[index] = value;
    }

    inline void set_primary(n_leaves_t value){

        primary_key_ = value;
    }

    inline n_leaves_t read_primary(){

        return primary_key_;
    }

    inline symbol_t leaf_to_symbol(level_t level) {
        
        symbol_t result = 0;
        dimension_t dimension = DATA_DIMENSION;

        for (size_t i = 0; i < dimension; i++) 
        {
            if (dimension_to_num_bits[i] <= level || level < start_dimension_bits[i])
                continue;

            level_t offset = dimension_to_num_bits[i] - level - 1;
            point_t coordinate = coordinates_[i];

            bool bit = GETBIT(coordinate, offset);
            result = (result << 1U) + bit;
        }
        return result;
    }    

    inline void update_symbol(data_point *end_range, symbol_t current_symbol, level_t level) {
        
        dimension_t dimension = DATA_DIMENSION;
        size_t visited_ct = 0;
        for (size_t j = 0; j < dimension; j++) {
            
            if (dimension_to_num_bits[j] <= level || level < start_dimension_bits[j])
                continue;

            visited_ct ++;

            level_t offset = dimension_to_num_bits[j] - level - 1U;    

            point_t start_coordinate = coordinates_[j];
            point_t end_coordinate = end_range->coordinates_[j];
            dimension_t symbol_offset = level_to_num_children[level] - visited_ct;

            bool start_bit = GETBIT(start_coordinate, offset); 
            bool end_bit = GETBIT(end_coordinate, offset);
            bool symbol_bit = GETBIT(current_symbol, symbol_offset);
            
            // Bring the start of the search range to second half
            if (symbol_bit && !start_bit) {

                start_coordinate = start_coordinate & compressed_bitmap::low_bits_unset[offset];
                SETBIT(start_coordinate, offset);
            } 
            // Bring the end of the search range to first half
            if (!symbol_bit && end_bit) {
         
                end_coordinate = end_coordinate | compressed_bitmap::low_bits_set[offset];
                CLRBIT(end_coordinate, offset);            
            }
            coordinates_[j] = start_coordinate;
            end_range->coordinates_[j] = end_coordinate;
        }
    }

private:
    point_t coordinates_[DATA_DIMENSION] = {0};
    n_leaves_t primary_key_ = 0;
};

#endif //MD_TRIE_DATA_POINT_H
