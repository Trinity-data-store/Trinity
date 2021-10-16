#ifndef MD_TRIE_POINT_ARRAY_H
#define MD_TRIE_POINT_ARRAY_H

#include "data_point.h"

class point_array {
public:
    explicit point_array(){
        capacity_ = 1;
        n_nodes_ = 0;
        points_ = (data_point **)malloc(capacity_ * sizeof(data_point *));
    }

    void add_leaf(data_point *p) {
        points_[n_nodes_] = p;
        n_nodes_ += 1;
        if (n_nodes_ == capacity_){
            capacity_ *= 2;
            points_ = (data_point **)realloc(points_, capacity_ * sizeof(data_point *));
        }
    }

    void reset() {

        free(points_);
        capacity_ = 1;
        n_nodes_ = 0;
        points_ = (data_point **)malloc(capacity_ * sizeof(data_point *));
    }

    inline n_leaves_t size() const {
        return n_nodes_;
    }

    inline data_point *at(size_t i) {
        return points_[i];
    }

private:
    data_point **points_;
    n_leaves_t capacity_;
    n_leaves_t n_nodes_; 
};


#endif //MD_TRIE_POINT_ARRAY_H
