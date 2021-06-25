#ifndef MD_TRIE_POINT_ARRAY_H
#define MD_TRIE_POINT_ARRAY_H

#include "data_point.h"

template<dimension_t DIMENSION>
class point_array {
public:
    explicit point_array(){
        capacity = 1;
        n_nodes = 0;
        points_ = (data_point<DIMENSION> **)malloc(capacity * sizeof(data_point<DIMENSION> *));
    }

    void add_leaf(data_point<DIMENSION> *p) {
        points_[n_nodes] = p;
        n_nodes += 1;
        if (n_nodes == capacity){
            capacity *= 2;
            points_ = (data_point<DIMENSION> **)realloc(points_, capacity * sizeof(data_point<DIMENSION> *));
        }
    }

    void reset() {

        free(points_);
        capacity = 1;
        n_nodes = 0;
        points_ = (data_point<DIMENSION> **)malloc(capacity * sizeof(data_point<DIMENSION> *));
    }

    inline n_leaves_t size() const {
        return n_nodes;
    }

    inline data_point<DIMENSION> *at(size_t i) {
        return points_[i];
    }

private:
    data_point<DIMENSION> **points_;
    n_leaves_t capacity;
    n_leaves_t n_nodes; 
};


#endif //MD_TRIE_POINT_ARRAY_H
