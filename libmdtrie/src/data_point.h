#ifndef MD_TRIE_DATA_POINT_H
#define MD_TRIE_DATA_POINT_H

#include "defs.h"
#include <cstdlib>
#include <cstring>
#include "compressed_bitmap.h"

class data_point {

public:

    explicit data_point(dimension_t num_dimensions){

        coordinates_ = std::vector<point_t>(num_dimensions, 0);
    };

    inline std::vector<point_t> get(){

        return coordinates_;
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

    inline symbol_t leaf_to_symbol(level_t level, level_t max_depth) {

        symbol_t result = 0;

        for (size_t i = 0; i < DIMENSION; i++) {
            point_t coordinate = coordinates_[i] >> level;
            bool bit = coordinate & 1;
            if (bit)
                result = (result << 1U) + bit;
        }
        return result;
    }    

    inline void update_symbol(data_point*end_range, symbol_t current_morton, level_t level, level_t max_depth) {

        // TODO
    }

private:

    std::vector<point_t> coordinates_;
    n_leaves_t primary_key_ = 0;

};

#endif //MD_TRIE_DATA_POINT_H
