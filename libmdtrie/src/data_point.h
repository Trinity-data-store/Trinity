#ifndef MD_TRIE_DATA_POINT_H
#define MD_TRIE_DATA_POINT_H

#include "defs.h"
#include <cstdlib>
#include <cstring>
#include "compressed_bitmap.h"
#include <bit>
#include <bitset>

class data_point {

public:
    
    explicit data_point(){
        coordinates_ = std::vector<point_t>(level_to_num_children[0], 0);
    }

    explicit data_point(std::vector<point_t> coordinates){
        coordinates_ = coordinates;
    }

    explicit data_point(dimension_t num_dimensions){

        coordinates_ = std::vector<point_t>(num_dimensions, 0);
    };

    inline std::vector<point_t> get(){

        return coordinates_;
    }

    inline void set(std::vector<point_t> coordinates){
        coordinates_ = coordinates;
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

    // Coordinates are pre-reversed!
    inline symbol_t leaf_to_symbol(level_t level) {

        symbol_t result = 0;
        dimension_t dimension = coordinates_.size();

        for (size_t i = 0; i < dimension; i++) {

            if (dimension_to_num_bits[i] <= level)
                continue;

            level_t offset = dimension_to_num_bits[i] - level - 1;


            point_t coordinate = coordinates_[i];

            // int offset = -1;
            // if (coordinate > 0)
            //     offset = dimension_to_num_bits[i] - level - 1U;
            //     // offset = bit_width(coordinate) - level - 1U;

            // bool bit = 0;

            // if (offset >= 0){
            //     point_t shifted_coordinate = coordinate >> offset;
            //     bit = shifted_coordinate & 1;
            // }
            bool bit = GETBIT(coordinate, offset);
            result = (result << 1U) + bit;
        }

        return result;
    }    

    inline void update_symbol(data_point *end_range, symbol_t current_symbol, level_t level) {
        
        dimension_t dimension = coordinates_.size();
        for (size_t j = 0; j < dimension; j++) {

            if (dimension_to_num_bits[j] <= level)
                continue;

            level_t offset = dimension_to_num_bits[j] - level - 1U;    

            point_t start_coordinate = coordinates_[j];
            point_t end_coordinate = end_range->coordinates_[j];
            dimension_t symbol_offset = dimension - j - 1;

            bool start_bit = GETBIT(start_coordinate, offset); 
            bool end_bit = GETBIT(end_coordinate, offset);
            bool symbol_bit = GETBIT(current_symbol, symbol_offset);
            
            // Bring the start of the search range to second half
            if (symbol_bit && !start_bit) {
                // point_t mask = (start_coordinate >> (level + 1)) << (level + 1);
                // start_coordinate -= mask;
                // SETBIT(start_coordinate, level);
                start_coordinate = start_coordinate & compressed_bitmap::low_bits_unset[offset];
                SETBIT(start_coordinate, offset);

            } 
            // Bring the end of the search range to first half
            if (!symbol_bit && end_bit) {
                // point_t mask = ((end_coordinate >> (level + 1)) | compressed_bitmap::low_bits_set[0]) << (level + 1); 
                // end_coordinate = end_coordinate | mask;
                // CLRBIT(end_coordinate, level);            
                end_coordinate = end_coordinate | compressed_bitmap::low_bits_set[offset];
                CLRBIT(end_coordinate, offset);            

            }
            coordinates_[j] = start_coordinate;
            end_range->coordinates_[j] = end_coordinate;
        }
    }

private:

    std::vector<point_t> coordinates_;
    n_leaves_t primary_key_ = 0;

};

#endif //MD_TRIE_DATA_POINT_H
