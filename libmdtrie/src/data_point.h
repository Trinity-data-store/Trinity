#ifndef MD_TRIE_DATA_POINT_H
#define MD_TRIE_DATA_POINT_H

#include "defs.h"
#include <cstdlib>
#include <cstring>
#include "compressed_bitmap.h"

class data_point {

public:
    
    explicit data_point(){
        coordinates_ = std::vector<point_t>(level_to_num_children[0], 0);
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

    inline symbol_t leaf_to_symbol(level_t level) {

        symbol_t result = 0;
        dimension_t dimension = coordinates_.size();
        for (size_t i = 0; i < dimension; i++) {
            point_t coordinate = coordinates_[i] >> level;
            bool bit = coordinate & 1;
            if (bit)
                result = (result << 1U) + bit;
        }
        return result;
    }    

    inline void update_symbol(data_point *end_range, symbol_t current_morton, level_t level, level_t max_depth) {

        level_t offset = max_depth - level - 1U;    
        for (size_t j = 0; j < level_to_num_children[level]; j++) {
            point_t start_coordinate = coordinates_[j];
            point_t end_coordinate = end_range->coordinates_[j];
            dimension_t morton_offset = level_to_num_children[level] - j - 1U;

            bool start_bit = GETBIT(start_coordinate, offset); 
            bool end_bit = GETBIT(end_coordinate, offset);
            bool morton_bit = GETBIT(current_morton, morton_offset);
            
            // Bring the start of the search range to second half
            if (morton_bit && !start_bit) {
                start_coordinate = start_coordinate & compressed_bitmap::low_bits_unset[offset];
                SETBIT(start_coordinate, offset);
            } 
            // Bring the end of the search range to first half
            if (!morton_bit && end_bit) {
                end_coordinate = end_coordinate | compressed_bitmap::low_bits_set[offset];
                // In the end, only the start_coordinate is kept as the returned point
                CLRBIT(end_coordinate, offset);            
            }
            // if (GETBIT(start_coordinate, offset) != GETBIT(end_coordinate, offset)){
            //     raise(SIGINT);
            // }
            coordinates_[j] = start_coordinate;
            end_range->coordinates_[j] = end_coordinate;
        }
    }

private:

    std::vector<point_t> coordinates_;
    n_leaves_t primary_key_ = 0;

};

#endif //MD_TRIE_DATA_POINT_H
