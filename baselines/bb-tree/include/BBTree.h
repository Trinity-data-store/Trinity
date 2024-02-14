#ifndef BBTREE
#define BBTREE
#pragma once

// Maximum bucket size
#define BUCKET_MAX 2500
// Average bucket size; used when rebuilding BBTREE (10% of b_max)
#define BUCKET_AVG 250
// k-1
#define DELIMITERS_PER_SPLIT 16
// Percentage of buckets that are allowed to be a superbucket
#define ALLOWED_SUPER_BUCKETS 0.01
// Percentage of buckets that are allowed to be sparse
#define ALLOWED_EMPTY_BUCKETS 0.2
// Percentage of data objects used as samples when rebuilding
#define REBUILD_SAMPLE_SIZE 0.1
// Number of historical queries used for adaptation
#define MONITOR_WORKLOAD_WINDOW 100
// Size of super buckets (=k)
#define SUPER_BUCKET_SIZE 17
// If super buckets contain less than SUPER_BUCKET_FILL_DEGREE * BUCKET_MAX
// objects, they morph into regular buckets
#define SUPER_BUCKET_FILL_DEGREE 0.5

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

// https://github.com/vit-vit/CTPL/
#include "ctpl_stl.h"

#include "BBTreeBucket.h"

/**
 * Used to order data objects dimensionwise.
 */
struct DimensionCompare {
  DimensionCompare(const unsigned int dimension)
    : dim(dimension) {}

  bool operator()(const std::vector<float> &a,
                  const std::vector<float> &b) const {
    return (a[dim] < b[dim]);
  }

  unsigned int dim;
};

/**
 * The BBTREE index structure.
 * Example usage with a 10-dimensional feature space:
 *   BBTree* bbtree = new BBTree(10);
 *
 * By default BBTree uses all available hardware threads for multi-threading.
 *
 * Example usage with a 10-dimensional feature space and 5 threads:
 *   BBTree* bbtree = new BBTree(10, 5);
 */
class BBTree {
 public:
   BBTree(size_t dimensions) :
     BBTree(dimensions, std::thread::hardware_concurrency()) {}

   BBTree(size_t dimensions, size_t num_threads) :
     dimensions(dimensions), num_threads(num_threads) {
     this->count = 0;
     this->num_buckets = 1;
     this->num_super_buckets = 0;
     this->num_empty_buckets = 0;
     this->height = 1;
     this->thread_pool = new ctpl::thread_pool(num_threads);
     this->buckets = new BBTreeBucket*[this->num_buckets];
     for (size_t i = 0; i < num_buckets; ++i)
       this->buckets[i] = new BBTreeRegularBucket();
     this->delimiter_dimensions = new int[1];
     this->delimiter_dimensions[0] = 0;
     this->delimiter_values = new float[DELIMITERS_PER_SPLIT];
     for (size_t i = 0; i < DELIMITERS_PER_SPLIT; ++i)
       delimiter_values[i] = std::numeric_limits<float>::max();
   };

   ~BBTree() {
     delete this->thread_pool;
     delete [] this->buckets;
     delete [] this->delimiter_dimensions;
     delete [] this->delimiter_values;
   };

   void printStatistics() const;
   size_t getCount() const;
   void InsertObject(const std::vector<float> feature_vector,
                     const uint32_t object_id);
   void BulkInsert(const std::vector<std::vector<float> > &feature_vectors,
                   const std::vector<uint32_t> &object_ids);
   bool DeleteObject(const std::vector<float> &feature_vector);
   uint32_t SearchObject(const std::vector<float> &search_object) const;
   std::vector<uint32_t> SearchRange(const std::vector<float> &lower_boundary,
                                     const std::vector<float> &upper_boundary);
   std::vector<uint32_t> SearchRangeMT(const std::vector<float> &lower_boundary,
                                       const std::vector<float> &upper_boundary);
   std::vector<uint32_t> SearchFixedRadiusNN(const std::vector<float> &search_object,
                                             const float &r);
   static void ScanBuckets(int thread_id,
                           BBTree *bbtree,
                           std::vector<uint32_t> &results,
                           const std::vector<float> &lower_boundary,
                           const std::vector<float> &upper_boundary,
                           const std::vector<size_t> &buckets,
                           const size_t start,
                           const size_t end);
   void RebuildDelimiters();

 private:
  size_t count;
  size_t dimensions;
  size_t num_buckets;
  size_t num_super_buckets;
  size_t num_empty_buckets;
  size_t num_threads;
  size_t height;
  int* delimiter_dimensions;
  float* delimiter_values;
  BBTreeBucket** buckets;
  // thread pool used by the parallel BBTREE to enable reuse of POSIX threads
  ctpl::thread_pool *thread_pool;
  // historical lower boundaries of range queries
  std::vector<std::vector<float> > last_lower_bounds;
  // historical upper boundaries of range queries
  std::vector<std::vector<float> > last_upper_bounds;

  size_t getNumberOfNodesInTreeOfHeight(const size_t height) const;
  inline std::vector<size_t> getBucketOfFeatureVector(const std::vector<float> &feature_vector) const;
  inline size_t getBucketOfFeatureVectorForInsert(const std::vector<float> &feature_vector,
                                                  const bool debug) const;
  inline std::vector<size_t> getBucketsForRange(const std::vector<float> &lower_boundary,
                                                const std::vector<float> &upper_boundary) const;
  inline void transformRegularIntoSuperBucket(const size_t bucket_id);
  inline void transformSuperIntoRegularBucket(const size_t bucket_id);
};

#endif
