#ifndef MD_TRIE_POINT_ARRAY_H
#define MD_TRIE_POINT_ARRAY_H

#include "data_point.h"

class point_array {
public:
    explicit point_array(){
        capacity = 1;
        n_nodes = 0;
        points_ = (data_point **)malloc(capacity * sizeof(data_point *));
    }

    void add_leaf(data_point *p) {
        points_[n_nodes] = p;
        n_nodes += 1;
        if (n_nodes == capacity){
            capacity *= 2;
            points_ = (data_point **)realloc(points_, capacity * sizeof(data_point *));
        }
        // points_.push_back(p);
    }

    void reset() {

        free(points_);
        capacity = 1;
        n_nodes = 0;
        points_ = (data_point **)malloc(capacity * sizeof(data_point *));
        // points_.clear();
    }

    inline n_leaves_t size() const {
        return n_nodes;
        // return points_.size();
    }

    inline data_point *at(size_t i) {
        return points_[i];
    }

private:
    data_point **points_;
    n_leaves_t capacity;
    n_leaves_t n_nodes; 
    // std::vector<data_point *> points_;
};


#endif //MD_TRIE_POINT_ARRAY_H
