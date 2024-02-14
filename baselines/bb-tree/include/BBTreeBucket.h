#ifndef BBTREEBUCKET
#define BBTREEBUCKET
#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdlib.h>
#include <vector>

// https://github.com/vit-vit/CTPL/
#include "ctpl_stl.h"

/**
 * Base class for buckets.
 */
class BBTreeBucket {
  public:
    virtual ~BBTreeBucket() {}
    virtual bool IsRegularBucket() const = 0;
    virtual bool IsFull(const size_t max_size) const = 0;
    virtual std::vector<float> GetObject(const size_t index) const = 0;
    virtual std::vector<float> GetRandomObject() const = 0;
    virtual uint32_t GetTid(const size_t index) const = 0;
    virtual size_t GetNumberOfObjects() const = 0;
    virtual void InsertObject(const std::vector<float> feature_vector,
                              const uint32_t object_id) = 0;
    virtual void BulkInsert(const std::vector<std::vector<float> > &feature_vectors,
                            const std::vector<uint32_t> &object_ids,
                            const size_t start,
                            const size_t end) = 0;
    virtual bool DeleteObject(const std::vector<float> feature_vector) = 0;
    virtual int32_t SearchObject(const std::vector<float> &search_object) const = 0;
    virtual void SearchRange(std::vector<uint32_t> &results,
                             const std::vector<float> &lower_boundary,
                             const std::vector<float> &upper_boundary) = 0;
  private:
    std::vector<std::vector<float> > data_objects;
    std::vector<uint32_t> tids;
};

/**
 * Regular bucket that can hold up to BUCKET_MAX data objects.
 */
class BBTreeRegularBucket : public BBTreeBucket {
  public:
    BBTreeRegularBucket() : count(0) {}
    bool IsRegularBucket() const;
    bool IsFull(const size_t max_size) const;
    std::vector<float> GetObject(const size_t index) const;
    std::vector<float> GetRandomObject() const;
    uint32_t GetTid(const size_t index) const;
    size_t GetNumberOfObjects() const;
    std::vector<std::vector<float> >& GetDataObjects();
    std::vector<uint32_t>& GetTids();
    void InsertObject(const std::vector<float> feature_vector,
                      const uint32_t object_id);
    void BulkInsert(const std::vector<std::vector<float> > &feature_vectors,
                    const std::vector<uint32_t> &object_ids,
                    const size_t start,
                    const size_t end);
    bool DeleteObject(const std::vector<float> feature_vector);
    int32_t SearchObject(const std::vector<float> &search_object) const;
    void SearchRange(std::vector<uint32_t> &results,
                     const std::vector<float> &lower_boundary,
                     const std::vector<float> &upper_boundary);
  private:
    size_t count;
    std::vector<std::vector<float> > data_objects;
    std::vector<uint32_t> tids;
};

/**
 * Superbucket that consists of z regular buckets.
 */
class BBTreeSuperBucket : public BBTreeBucket {
  public:
    BBTreeSuperBucket(size_t num_buckets,
                     size_t delimiter_dimension,
                     float* delimiter_values) :
      num_buckets(num_buckets),
      delimiter_dimension(delimiter_dimension),
      delimiter_values(delimiter_values),
      data_objects(new std::vector<std::vector<float> >[num_buckets]),
      tids(new std::vector<uint32_t>[num_buckets]) {
      this->count = 0;
    }

    ~BBTreeSuperBucket() {
      delete [] this->delimiter_values;
      delete [] this->data_objects;
      delete [] this->tids;
    }

    bool IsRegularBucket() const;
    bool IsFull(const size_t max_size) const;
    std::vector<float> GetObject(const size_t index) const;
    std::vector<float> GetRandomObject() const;
    uint32_t GetTid(const size_t index) const;
    size_t GetNumberOfObjects() const;
    std::vector<std::vector<float> >& GetDataObjects(const size_t bucket_id);
    std::vector<uint32_t>& GetTids(const size_t bucket_id);
    void InsertObject(const std::vector<float> feature_vector,
                      const uint32_t object_id);
    void BulkInsert(const std::vector<std::vector<float> > &feature_vectors,
                    const std::vector<uint32_t> &object_ids,
                    const size_t start,
                    const size_t end);
    bool DeleteObject(const std::vector<float> feature_vector);
    int32_t SearchObject(const std::vector<float> &search_object) const;
    void SearchRange(std::vector<uint32_t> &results,
                     const std::vector<float> &lower_boundary,
                     const std::vector<float> &upper_boundary);
  private:
    size_t count;
    size_t num_buckets;
    size_t delimiter_dimension;
    float* delimiter_values;
    std::vector<std::vector<float> > *data_objects;
    std::vector<uint32_t> *tids;

    size_t getBucket(const std::vector<float> &feature_vector) const;
};

#endif
