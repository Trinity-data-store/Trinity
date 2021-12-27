#ifndef MD_TRIE_POINT_ARRAY_H
#define MD_TRIE_POINT_ARRAY_H

#include "data_point.h"

template<dimension_t DIMENSION>
class point_array {
public:
    explicit point_array(){}

    void add_leaf(data_point<DIMENSION> *p) {
        points_.push_back(p);
    }

    void reset() {
        if (points_.size() > 0)
            points_.clear();
    }

    inline n_leaves_t size() const {
        return points_.size();
    }

    inline data_point<DIMENSION> *at(size_t i) {
        return points_[i];
    }

private:
    std::vector<data_point<DIMENSION> *> points_;
};

#endif //MD_TRIE_POINT_ARRAY_H
