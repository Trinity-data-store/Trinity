#ifndef MD_TRIE_POINT_ARRAY_H
#define MD_TRIE_POINT_ARRAY_H

#include "data_point.h"

class point_array {
public:
    point_array() = default;

    void add_leaf(data_point *p) {
        points_.push_back(p);
    }

    void reset() {
        points_.clear();
    }

    size_t size() const {
        return points_.size();
    }

    data_point *at(size_t i) {
        return points_.at(i);
    }

private:
    std::vector<data_point *> points_;
};


#endif //MD_TRIE_POINT_ARRAY_H
