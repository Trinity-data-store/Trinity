#include "BBTree.h"

/**
 * BBTree::printStatistics() prints statistics, like tree height,
 * number of buckets, or the bucket sizes.
 */
void BBTree::printStatistics() const {
  std::cout << "Tree Height: " << this->height << " Buckets: " <<
               this->num_buckets << std::endl;
  std::cout << "Bucket sizes:" << std::endl;
  for (size_t i = 0; i < this->num_buckets; ++i) {
    std::cout << i << ":" << this->buckets[i]->GetNumberOfObjects() << " ";
    if (i != 0 && i % 24 == 0) {
      std::cout << std::endl;
    }
  }
  std::cout << std::endl;
}

/**
 * BBTree::getCount() returns the total number of data objects stored in the
 * BB-Tree instance.
 */
size_t BBTree::getCount() const {
  return this->count;
}

/**
 * BBTree::InsertObject(feature_vector,id) inserts the data object with the
 * given identifier into the BB-Tree instance.
 * It may invoke a rebuild of BB-Tree given that a superbucket overflows or too
 * many superbuckets exist.
 */
void BBTree::InsertObject(const std::vector<float> feature_vector,
                         const uint32_t object_id) {
  // check that the new data object matches the dimensionality of the feature space
  assert(feature_vector.size() == this->dimensions);

  // get the bucket that the new data object is inserted into
  const size_t matching_bucket = this->getBucketOfFeatureVectorForInsert(feature_vector,
                                                                         false);
  // insert into the bucket
  this->buckets[matching_bucket]->InsertObject(feature_vector, object_id);
  // increase global data object counter
  this->count++;

  // check if bucket overflows
  if (this->buckets[matching_bucket]->IsFull(BUCKET_MAX)) {
    // if it is a regular bucket, try to transform it into a superbucket
    if (this->buckets[matching_bucket]->IsRegularBucket()) {
      // if too many superbuckets exist, invoke a rebuild
      if (this->num_super_buckets++ >
          (ALLOWED_SUPER_BUCKETS * this->num_buckets)) {
        this->RebuildDelimiters();
      } else {
        this->transformRegularIntoSuperBucket(matching_bucket);
      }
    } else { // invoke a rebuild as the overflowing bucket is a superbucket
      this->RebuildDelimiters();
    }
  }
}

/**
 * BBTree::BulkInsert(feature_vectors,ids) inserts a set of data objects with the
 * given identifiers into the BB-Tree instance.
 */
void BBTree::BulkInsert(const std::vector<std::vector<float> > &feature_vectors,
                       const std::vector<uint32_t> &object_ids) {
  assert(feature_vectors.size() > 0);
  // check that the new data object matches the dimensionality of the feature space
  assert(feature_vectors[0].size() == this->dimensions);
  // do not allow bulk inserts on already built index structures
  assert(this->count == 0);

  this->count = feature_vectors.size();
  // insert batches of feature vectors into buckets
  this->num_buckets = (feature_vectors.size() / BUCKET_MAX) + 1;
  this->buckets = new BBTreeBucket*[this->num_buckets];
  for (size_t i = 0; i < num_buckets; ++i)
    this->buckets[i] = new BBTreeRegularBucket();
  const size_t partition_size = feature_vectors.size() / this->num_buckets;

  // batch-wise insertions
  for (size_t i = 0; i < this->num_buckets; ++i) {
    const size_t start = i * partition_size;
    const size_t end = (i == this->num_buckets - 1) ? feature_vectors.size() : ((i+1) * partition_size);
    this->buckets[i]->BulkInsert(feature_vectors, object_ids, start, end);
  }

  this->RebuildDelimiters();
}

/**
 * BBTree::DeleteObject(feature_vector) deletes the given data object from
 * BB-Tree.
 * It may invoke a rebuild of BB-Tree if too many sparse buckets exist.
 */
bool BBTree::DeleteObject(const std::vector<float> &feature_vector) {
  // get the buckets that may hold the to-be-deleted data object
  std::vector<size_t> buckets = this->getBucketOfFeatureVector(feature_vector);

  // iterate over all relevant buckets and search for the to-be-deleted object
  for (size_t i = 0; i < buckets.size(); ++i) {
    if (this->buckets[buckets[i]]->DeleteObject(feature_vector)) {
      this->count--;
      // increase the sparse buckets counter
      if (this->buckets[buckets[i]]->GetNumberOfObjects() == 0) {
        this->num_empty_buckets++;
      }
      // invoke a rebuild if too many sparse buckets exist
      if (this->num_empty_buckets >=
          this->num_buckets * ALLOWED_EMPTY_BUCKETS) {
        this->RebuildDelimiters();
      // or transform underflowing super bucket into regular bucket
      } else if (this->buckets[buckets[i]]->IsRegularBucket() == false &&
                 this->buckets[buckets[i]]->GetNumberOfObjects() <
                   (BUCKET_MAX * SUPER_BUCKET_FILL_DEGREE)) {
        this->transformSuperIntoRegularBucket(buckets[i]);
      }
      // data object has been successfully deleted
      return true;
    }
  }

  // data object could not be found
  return false;
}

/**
 * BBTree::SearchObject(feature_vector) returns the identifier of the
 * specified data object.
 * If no matching data object has been found, it returns -1.
 */
uint32_t BBTree::SearchObject(const std::vector<float> &feature_vector) const {
  // get the buckets that may hold the searched data object
  const std::vector<size_t> buckets = this->getBucketOfFeatureVector(feature_vector);
  int32_t result;

  // iterate over all relevant buckets and search for the data object
  for (size_t i = 0; i < buckets.size(); ++i) {
    result = this->buckets[buckets[i]]->SearchObject(feature_vector);
    if (result != -1) {
      // match
      return result;
    }
  }

  // no matching data object has been found
  return -1;
}

/**
 * BBTree::SearchRange(lower_boundary, upper_boundary) executes the specified
 * range query.
 * It returns a std::vector containing all the tids of the matching objects.
 */
std::vector<uint32_t> BBTree::SearchRange(const std::vector<float> &lower_boundary,
                                         const std::vector<float> &upper_boundary) {
  std::vector<uint32_t> results;
  // buckets that are relevant for the given range query
  const std::vector<size_t> match_buckets = this->getBucketsForRange(lower_boundary,
                                                                     upper_boundary);
  const size_t num_buckets = match_buckets.size();
  for (size_t bucket = 0; bucket < num_buckets; ++bucket) {
    this->buckets[match_buckets[bucket]]->SearchRange(results,
                                                      lower_boundary,
                                                      upper_boundary);
  }

  // monitor query workload
  this->last_lower_bounds.push_back(lower_boundary);
  this->last_upper_bounds.push_back(upper_boundary);

  return results;
}

/**
 * BBTree::SearchRangeMT(lower_boundary, upper_boundary) executes the specified
 * range query in parallel using multi-threading.
 * It returns a std::vector containing all the tids of the matching objects.
 */
std::vector<uint32_t> BBTree::SearchRangeMT(const std::vector<float> &lower_boundary,
                                           const std::vector<float> &upper_boundary) {
  std::vector<uint32_t> results;
  const std::vector<size_t> match_buckets = this->getBucketsForRange(lower_boundary,
                                                                     upper_boundary);
  const size_t num_buckets = match_buckets.size();
  // take the current thread into account
  const size_t dop = ((num_buckets < this->num_threads) ? num_buckets : this->num_threads) - 1;

  std::vector<size_t> partitions;
  for (size_t i = 0; i < num_buckets; ++i) {
    if (this->buckets[match_buckets[i]]->IsRegularBucket()) { // regular bucket
      partitions.push_back(match_buckets[i]);
    } else { // super bucket
      partitions.push_back(match_buckets[i]);
      partitions.push_back(match_buckets[i]);
    }
  }

  //const size_t partition_size = num_buckets / (dop + 1);
  const size_t partition_size = partitions.size() / (dop + 1);
  std::future<void> *futures = new std::future<void>[dop];
  std::vector<std::vector<uint32_t> > thread_results(dop,
                                                     std::vector<uint32_t>());
  size_t start, end;
  for (size_t i = 0; i < dop; ++i) {
    start = (i * partition_size);
    // compare with end of previous partition to handle
    // superbuckets spanning multiple partitions
    while (start > 0 && partitions[start] == partitions[end-1]) {
      start++;
    }
    end = ((i+1) * partition_size);
    // ensure that partition is not empty
    futures[i] = this->thread_pool->push(std::ref(BBTree::ScanBuckets),
                                         this,
                                         std::ref(thread_results[i]),
                                         lower_boundary,
                                         upper_boundary,
                                         std::ref(partitions),
                                         start,
                                         end);
  }

  // do something useful with this thread :-)
  start = (dop * partition_size);
  // compare with end of previous partition to handle
  // superbuckets spanning multiple partitions
  while (start > 0 && partitions[start] == partitions[end-1]) {
    start++;
  }
  end = partitions.size();
  // ensure that partition is not empty
  if(end > start) {
    BBTree::ScanBuckets(0,
                       this,
                       std::ref(results),
                       lower_boundary,
                       upper_boundary,
                       std::ref(partitions),
                       start,
                       end);
  }

  // monitor query workload
  this->last_lower_bounds.push_back(lower_boundary);
  this->last_upper_bounds.push_back(upper_boundary);

  // collect results from threads
  for (size_t i = 0; i < dop; ++i) {
    futures[i].get();
    results.insert(std::end(results),
                   std::begin(thread_results[i]),
                   std::end(thread_results[i]));
  }
  delete [] futures;

  return results;
}

/**
 * BBTree::ScanBuckets(id,bbtree,results,lower_bounds,upper_bounds,match_buckets,start,end)
 * executes a range query on the relevant buckets start to end.
 * The tids of the matching objects are stored in the std::vector results.
 *
 * It is solely used by the parallel BB-Tree.
 */
void BBTree::ScanBuckets(int thread_id,
                        BBTree *bbtree,
                        std::vector<uint32_t> &results,
                        const std::vector<float> &lower_boundary,
                        const std::vector<float> &upper_boundary,
                        const std::vector<size_t> &match_buckets,
                        const size_t start,
                        const size_t end) {
  for (size_t i = start; i < end; ++i) {
    // do not search multiple times in a superbucket
    if (i == start || match_buckets[i] != match_buckets[i-1]) {
      bbtree->buckets[match_buckets[i]]->SearchRange(results,
                                                    lower_boundary,
                                                    upper_boundary);
    }
  }
}

/**
 * BBTree::SearchFixedRadiusNN(search_object, r) returns all data objects that
 * are located within radius r with respect to the given search_object.
 * It returns a std::vector containing the tids of all matching objects.
 */
std::vector<uint32_t> BBTree::SearchFixedRadiusNN(const std::vector<float> &search_object,
                                                 const float &r) {
  // transform the rNN query into a conventional range query
  std::vector<float> lower(this->dimensions);
  std::vector<float> upper(this->dimensions);
  for (size_t i = 0; i < this->dimensions; ++i) {
    lower[i] = search_object[i] - r/2.0;
    upper[i] = search_object[i] + r/2.0;
  }

  return this->SearchRange(lower, upper);
}

/**
 * BBTree::getNumberOfNodesInTreeOfHeight(h) returns the number of
 * inner nodes in a k-ary tree of height h.
 * Implemented according to https://en.wikipedia.org/wiki/K-ary_tree
 */
size_t BBTree::getNumberOfNodesInTreeOfHeight(const size_t height) const {
  return ((pow((DELIMITERS_PER_SPLIT+1), height)-1) / (DELIMITERS_PER_SPLIT));
}

/**
 * BBTree::getBucketsForRange(lower_bounds,upper_bounds) returns the buckets
 * relevant for a given range query.
 */
inline std::vector<size_t> BBTree::getBucketsForRange(const std::vector<float> &lower_boundary,
                                                     const std::vector<float> &upper_boundary) const {
  std::vector<size_t> buckets;
  if (this->num_buckets == 1) {
    buckets.push_back(0);
    return buckets;
  }
  std::vector<size_t> positions(1,0);
  std::vector<std::vector<size_t> > rel_positions(1, std::vector<size_t>());

  for (size_t i = 0; i < this->height; ++i) {
    const size_t dimension = this->delimiter_dimensions[i];
    const size_t num_positions = positions.size();
    for (size_t j = 0; j < num_positions; ++j) {
        if (this->delimiter_values[positions[j]] >= lower_boundary[dimension]) {
          std::vector<size_t> new_rel_pos = rel_positions[j];
          new_rel_pos.push_back(0);
          size_t times = 0;
          size_t new_pos = 0;
          for (int m = new_rel_pos.size(); m > 0; m--) {
            new_pos += new_rel_pos[m - 1] * pow((DELIMITERS_PER_SPLIT+1), times++);
          }
          new_pos = (this->getNumberOfNodesInTreeOfHeight(i+1) + new_pos) *
                    DELIMITERS_PER_SPLIT;
          positions.push_back(new_pos);
          rel_positions.push_back(new_rel_pos);
        }
      for (size_t k = 1; k < DELIMITERS_PER_SPLIT; ++k) {
        if (this->delimiter_values[positions[j] + k-1] > upper_boundary[dimension]) {
          break;
        }
        if (this->delimiter_values[positions[j] + k] >= lower_boundary[dimension] &&
            this->delimiter_values[positions[j] + k - 1] <= upper_boundary[dimension]) {
          std::vector<size_t> new_rel_pos = rel_positions[j];
          new_rel_pos.push_back(k);
          size_t times = 0;
          size_t new_pos = 0;
          for (int m = new_rel_pos.size(); m > 0; m--) {
            new_pos += new_rel_pos[m - 1] *
                       pow((DELIMITERS_PER_SPLIT+1), times++);
          }
          new_pos = (this->getNumberOfNodesInTreeOfHeight(i+1) + new_pos) *
                    DELIMITERS_PER_SPLIT;
          positions.push_back(new_pos);
          rel_positions.push_back(new_rel_pos);
        }
      }
      if (this->delimiter_values[positions[j] + DELIMITERS_PER_SPLIT - 1] <
          upper_boundary[dimension]) {
          std::vector<size_t> new_rel_pos = rel_positions[j];
          new_rel_pos.push_back(DELIMITERS_PER_SPLIT);
          size_t times = 0;
          size_t new_pos = 0;
          for (int m = new_rel_pos.size(); m > 0; m--) {
            new_pos += new_rel_pos[m - 1] *
                       pow((DELIMITERS_PER_SPLIT+1), times++);
          }
          new_pos = (this->getNumberOfNodesInTreeOfHeight(i+1) + new_pos) *
                    DELIMITERS_PER_SPLIT;
          positions.push_back(new_pos);
          rel_positions.push_back(new_rel_pos);
      }
    }
    positions.erase(positions.begin(), positions.begin() + num_positions);
    rel_positions.erase(rel_positions.begin(),
                        rel_positions.begin() + num_positions);
  }

  for (size_t i = 0; i < positions.size(); ++i) {
    buckets.push_back((positions[i] -
                       (this->getNumberOfNodesInTreeOfHeight(this->height) *
                        DELIMITERS_PER_SPLIT)) /
                      DELIMITERS_PER_SPLIT);
  }

  return buckets;
}

/**
 * BBTree::getBucketOfFeatureVector(feature_vector) returns the buckets relevant
 * for a given point query object.
 */
inline std::vector<size_t> BBTree::getBucketOfFeatureVector(const std::vector<float> &feature_vector) const {
  std::vector<size_t> buckets;
  // single (super)bucket
  if (this->num_buckets == 1) {
    buckets.push_back(0);
    return buckets;
  }
  size_t bucket = 0;
  size_t rel_pos = 0;
  size_t add_buckets = 1;
  std::vector<size_t> rel_positions;

  for (size_t i = 0; i < this->height; ++i) {
    rel_pos = 0;
    const size_t dimension = this->delimiter_dimensions[i];
    size_t first   = 0;
    size_t last    = DELIMITERS_PER_SPLIT - 1;
    size_t middle  = 0;

    // binary search linearized inner node
    while (first < last) {
      middle = (first + last) / 2;
      if (this->delimiter_values[bucket + middle] <
          feature_vector[dimension]) {
        first = middle + 1;
      } else if (this->delimiter_values[bucket + middle] ==
                 feature_vector[dimension]) {
        rel_pos = middle;
        break;
      } else {
        last = middle - 1;
      }
    }
    if (first >= last) {
      rel_pos = first;
    }
    if (feature_vector[dimension] > this->delimiter_values[bucket + rel_pos]) {
      rel_pos++;
    }
    while (rel_pos > 0 &&
           rel_pos < DELIMITERS_PER_SPLIT &&
           this->delimiter_values[bucket + rel_pos] ==
           this->delimiter_values[bucket + rel_pos - 1]) {
      rel_pos--;
    }

    // check for duplicates on last level
    if (i == this->height - 1) {
      for (size_t k = 1; k < (DELIMITERS_PER_SPLIT - rel_pos); ++k) {
        if (this->delimiter_values[bucket + rel_pos] ==
            this->delimiter_values[bucket + rel_pos + k]) {
          add_buckets++;
        } else {
          break;
        }
      }
    }

    rel_positions.push_back(rel_pos);
    size_t level = 0;
    bucket = 0;
    // rel_positions is of size k (the variable)
    for (int m = rel_positions.size(); m > 0; m--) {
      bucket += rel_positions[m - 1] * pow((DELIMITERS_PER_SPLIT+1), level++);
    }
    bucket = (this->getNumberOfNodesInTreeOfHeight(i+1) + bucket) *
      DELIMITERS_PER_SPLIT;
  }
  bucket = (bucket - (this->getNumberOfNodesInTreeOfHeight(this->height)
                * DELIMITERS_PER_SPLIT))
            / DELIMITERS_PER_SPLIT;

  // consider multiple buckets on the last level
  for (size_t i = 0; i < add_buckets; ++i) {
    buckets.push_back(bucket + i);
  }

  return buckets;
}

/**
 * BBTree::getBucketOfFeatureVectorForInsert determines the bucket relevant for
 * inserting a given feature vector.
 * Determining the relevant bucket for an insert is less complex than for a
 * point query as only one bucket needs to be returned.
 */
inline size_t BBTree::getBucketOfFeatureVectorForInsert(const std::vector<float> &feature_vector,
                                                       const bool debug) const {
  size_t bucket = 0;
  if (this->num_buckets == 1)
    return bucket;

  std::vector<size_t> rel_positions;

  for (size_t i = 0; i < this->height; ++i) {
    const size_t dimension = this->delimiter_dimensions[i];
    size_t rel_pos = 0;
    if (i == (this->height - 1)) {
      int first = 0;
      int last = 0;
      for (size_t j = 0; j < DELIMITERS_PER_SPLIT; ++j) {
        if (feature_vector[dimension] <= this->delimiter_values[bucket]) {
          first = rel_pos;
          last = rel_pos;
          for (size_t k = 1; k < (DELIMITERS_PER_SPLIT - j); ++k) {
            if (this->delimiter_values[bucket] ==
                this->delimiter_values[bucket + k]) {
              last = rel_pos + k;
            } else {
              break;
            }
          }
          break;
        } else {
          bucket++;
          rel_pos++;
        }
      }
      if (rel_pos != DELIMITERS_PER_SPLIT) {
        if (last > first) {
          rel_pos = first + (rand() % (last-first));
        } else {
          rel_pos = first;
        }
      }
    } else {
      for (size_t j = 0; j < DELIMITERS_PER_SPLIT; ++j) {
        if (feature_vector[dimension] <= this->delimiter_values[bucket]) {
          break;
        } else {
          bucket++;
          rel_pos++;
        }
      }
    }
    rel_positions.push_back(rel_pos);
    size_t level = 0;
    bucket = 0;
    // rel_positions is of size k (the variable)
    for (int m = rel_positions.size(); m > 0; m--) {
      bucket += rel_positions[m - 1] * pow((DELIMITERS_PER_SPLIT+1), level++);
    }
    bucket = (this->getNumberOfNodesInTreeOfHeight(i+1) + bucket) *
      DELIMITERS_PER_SPLIT;
  }

  bucket = (bucket - (this->getNumberOfNodesInTreeOfHeight(this->height)
                * DELIMITERS_PER_SPLIT))
            / DELIMITERS_PER_SPLIT;

  return bucket;
}

/**
 * BBTree::transformRegularIntoSuperBucket(bucket_id) transforms an
 * overflowing BBTreeRegularBucket into a BBTreeSuperBucket.
 *
 * It determines a new delimiter dimension and z-1 delimiter values,
 * which are used to divide the data objects into the z new buckets.
 */
inline void BBTree::transformRegularIntoSuperBucket(const size_t bucket_id) {
  std::vector<std::vector<float> > data_objects;
  BBTreeBucket* bucket = this->buckets[bucket_id];

  for (size_t i = 0; i < bucket->GetNumberOfObjects(); ++i) {
    data_objects.push_back(bucket->GetObject(i));
    // preserve tid for sorting
    data_objects[i].push_back((float) bucket->GetTid(i));
  }

  // determine delimiter dimension
  std::vector<std::vector<int> > distinct_values =
    std::vector<std::vector<int> >(this->dimensions, std::vector<int>(2));
  for (size_t i = 0; i < this->dimensions; ++i) {
    std::vector<float> dim_values = std::vector<float>(data_objects.size());
    for (size_t j = 0; j < data_objects.size(); ++j) {
            dim_values[j] = data_objects[j][i];
    }
    std::sort(dim_values.begin(), dim_values.end());
    // get number of distinct values for dimension i
    distinct_values[i][0] = i;
    distinct_values[i][1] = std::unique(dim_values.begin(),
                                        dim_values.end())
                                - dim_values.begin();
  }
  // order dimensions by number of distinct values
  std::sort(distinct_values.begin(),
            distinct_values.end(),
            [](const std::vector< int >& a, const std::vector< int >& b)
              { return a[1] > b[1]; });
  // choose dimension with largest number of distinct values
  const size_t delimiter_dimension = (size_t) distinct_values[0][0];

  // determine delimiter values
  DimensionCompare cmp(delimiter_dimension);
  std::sort(data_objects.begin(), data_objects.end(), cmp);
  float* delimiter_values = new float[SUPER_BUCKET_SIZE - 1];
  const size_t range_size = data_objects.size() / SUPER_BUCKET_SIZE;
  for (size_t i = 1; i < SUPER_BUCKET_SIZE; ++i) {
    delimiter_values[i - 1] = data_objects[i * range_size][delimiter_dimension];
  }
  BBTreeBucket* new_bucket = new BBTreeSuperBucket(SUPER_BUCKET_SIZE,
                                             delimiter_dimension,
                                             delimiter_values);

  size_t tid = 0;
  // insert data objects into new superbucket
  for (size_t i = 0; i < data_objects.size(); ++i) {
    tid = data_objects[i][this->dimensions];
    data_objects[i].pop_back();
    new_bucket->InsertObject(data_objects[i], tid);
  }

  delete this->buckets[bucket_id];
  this->buckets[bucket_id] = new_bucket;
}

/**
 * BBTree::transformSuperIntoRegularBucket(bucket_id) transforms an
 * underflowing BBTreeSuperBucket into a BBTreeRegularBucket.
 */
inline void BBTree::transformSuperIntoRegularBucket(const size_t bucket_id) {
  BBTreeBucket* new_bucket = new BBTreeRegularBucket();

  for (size_t i = 0; i < SUPER_BUCKET_SIZE; ++i) {
    std::vector<std::vector<float> > &data_objects =
      ((BBTreeSuperBucket*) this->buckets[bucket_id])->GetDataObjects(i);
    std::vector<uint32_t> &tids =
      ((BBTreeSuperBucket*) this->buckets[bucket_id])->GetTids(i);
    for (size_t j = 0; j < data_objects.size(); ++j) {
      new_bucket->InsertObject(data_objects[j], tids[j]);
    }
  }

  delete this->buckets[bucket_id];
  this->buckets[bucket_id] = new_bucket;
}

/**
 * BBTree::RebuildDelimiters() rebuilds the inner nodes and bucket structure
 * according to the current data distribution determined by sampling.
 * If workload statistics are available, it reorganizes the delimiter
 * dimensions according to the single-dimension selectivities of the last
 * executed range queries.
 * This function is very huge and legacy; it may need a complete rewrite ;-)
 */
void BBTree::RebuildDelimiters() {
  // retrieve samples
  const size_t num_samples = this->count * REBUILD_SAMPLE_SIZE;
  std::vector<std::vector<float> > samples(num_samples);
  for (size_t i = 0; i < num_samples; ++i) {
    int rand_bucket;
    int bucket_size;
    while (true) {
      rand_bucket = rand () % this->num_buckets;
      bucket_size = this->buckets[rand_bucket]->GetNumberOfObjects();
      if (bucket_size > 0)
        break;
    }
    samples[i] = this->buckets[rand_bucket]->GetRandomObject();
  }
  std::vector<std::vector<double> > avg_selectivities(this->dimensions,
                                                     std::vector<double>(2, 0.0));
  for (size_t i = 0; i < this->dimensions; ++i) {
    avg_selectivities[i][0] = (double) i;
  }

  // if queries have been recorded, use them to determine the average
  // selectivities that the single dimensions are typically queried with
  if (this->last_lower_bounds.size() > 0) {
    size_t num_queries = 0;
    for (int i = this->last_lower_bounds.size() - 1; i >= 0; --i) {
      if (++num_queries >= MONITOR_WORKLOAD_WINDOW)
        break;
      for (size_t j = 0; j < samples.size(); ++j) {
        for (size_t k = 0; k < this->dimensions; ++k) {
          if (this->last_lower_bounds[i][k] <= samples[j][k] &&
              this->last_upper_bounds[i][k] >= samples[j][k]) {
            avg_selectivities[k][1] += 1.0;
          }
        }
      }
    }
    for (size_t i = 0; i < this->dimensions; ++i) {
      avg_selectivities[i][1] = (avg_selectivities[i][1] /
                                 (double) samples.size()) /
                                 (double) (num_queries-1);
    }
    // sort by average selectivity such that high selectivities are
    // moved to the beginning (top of the tree)
    std::sort(avg_selectivities.begin(),
              avg_selectivities.end(),
              [](const std::vector< double >& a, const std::vector< double >& b)
                { return a[1] < b[1]; });
  }

  // determine new number of buckets and new tree height
  size_t tmp_buckets = this->count / BUCKET_AVG;
  size_t new_height = 0;
  while (tmp_buckets > 0) {
    tmp_buckets = tmp_buckets / (DELIMITERS_PER_SPLIT+1);
    new_height++;
  }
  size_t new_num_delimiters = this->getNumberOfNodesInTreeOfHeight(new_height) *
                              DELIMITERS_PER_SPLIT;
  int* new_delimiter_dimensions = new int[new_height];
  float* new_delimiter_values = new float[new_num_delimiters];
  // if statistics about average selectivities exist,
  // order delimiter dimensions by them; otherwise use round robin
  if (this->last_lower_bounds.size() > 0) {
    for (size_t i = 0; i < new_height; ++i) {
      new_delimiter_dimensions[i] =
        (int) avg_selectivities[i % this->dimensions][0];
    }
  } else { // no statistics available: order by number of distinct values
		  std::vector<std::vector<int> > distinct_values =
				  std::vector<std::vector<int> >(this->dimensions, std::vector<int>(2));
		  for (size_t i = 0; i < this->dimensions; ++i) {
                  std::vector<float> dim_values = std::vector<float>(samples.size());
                  for (size_t j = 0; j < samples.size(); ++j) {
                          dim_values[j] = samples[j][i];
                  }
                  std::sort(dim_values.begin(), dim_values.end());

                  // get number of distinct values for dimension i
                  distinct_values[i][0] = i;
                  distinct_values[i][1] = std::unique(dim_values.begin(),
                                                      dim_values.end())
                                           - dim_values.begin();
		  }
		  std::sort(distinct_values.begin(), distinct_values.end(),
						    [](const std::vector< int >& a, const std::vector< int >& b)
						    { return a[1] > b[1]; });
      if (new_height > this->dimensions) {
              std::vector<std::vector<int> > dist_values_aligned =
                      std::vector<std::vector<int> >(new_height, std::vector<int>(2));
              for (size_t i = 0; i < new_height; ++i) {
                      dist_values_aligned[i] = distinct_values[i % this->dimensions];
              }
              std::sort(dist_values_aligned.begin(), dist_values_aligned.end(),
                              [](const std::vector< int >& a, const std::vector< int >& b)
                              { return a[1] > b[1]; });
              for (size_t i = 0; i < new_height; ++i) {
                      new_delimiter_dimensions[i] = dist_values_aligned[i][0];
              }
      } else {
              for (size_t i = 0; i < new_height; ++i) {
                      new_delimiter_dimensions[i] = distinct_values[i][0];
              }
      }
  }

  // determine new delimiter values
  size_t delim_value = 0;
  std::vector<int> delimiter_indexes;
  delimiter_indexes.push_back(samples.size());
  // sample with REBUILD_SAMPLE_SIZE % of all values
  for (size_t i = 0; i < new_height; ++i) {
    size_t start = 0;
    DimensionCompare cmp(new_delimiter_dimensions[i]);
    size_t num_delimiter_indexes = delimiter_indexes.size();
    for (size_t j = 0; j < num_delimiter_indexes; ++j) {
      // sort current subset according to current dimension
      std::sort(samples.begin() + start,
                samples.begin() + (delimiter_indexes[j]),
                cmp);

      // find delimiter values
      int range_size = (delimiter_indexes[j] - start) / (DELIMITERS_PER_SPLIT+1);
      for (size_t k = 1; k <= DELIMITERS_PER_SPLIT; ++k) {
        int l = 0;
        new_delimiter_values[delim_value++] =
          samples[start + k*range_size][new_delimiter_dimensions[i]];
        delimiter_indexes.push_back(start + k*range_size + l);
      }
      delimiter_indexes.push_back(delimiter_indexes[j]);
      start = delimiter_indexes[j];
    }
    delimiter_indexes.erase(delimiter_indexes.begin(),
                            delimiter_indexes.begin() + num_delimiter_indexes);
  }

  // re-partition data objects to new buckets
  size_t new_num_buckets = this->getNumberOfNodesInTreeOfHeight(new_height+1) -
                           this->getNumberOfNodesInTreeOfHeight(new_height);
  BBTreeBucket** new_buckets = new BBTreeBucket*[new_num_buckets];
  for (size_t i = 0; i < new_num_buckets; ++i) {
    new_buckets[i] = new BBTreeRegularBucket();
  }

  // traverse over "old" buckets and copy data objects into new buckets
  for (size_t i = 0; i < this->num_buckets; ++i) {
    if (this->buckets[i]->IsRegularBucket()) { // regular bucket
      std::vector<std::vector<float> > &data_objects =
        ((BBTreeRegularBucket*) this->buckets[i])->GetDataObjects();
      std::vector<uint32_t> &tids =
        ((BBTreeRegularBucket*) this->buckets[i])->GetTids();
      for (size_t j = 0; j < data_objects.size(); ++j) {
        // determine new bucket loation
        size_t cur_pos = 0;
        std::vector<size_t> rel_positions;
        for (size_t k = 0; k < new_height; ++k) {
          size_t rel_pos = 0;
          if (k == (new_height - 1)) {
            int first = 0;
            int last = 0;
            for (size_t l = 0; l < DELIMITERS_PER_SPLIT; ++l) {
              if (data_objects[j][new_delimiter_dimensions[k]] <=
                  new_delimiter_values[cur_pos]) {
                first = rel_pos;
                last = rel_pos;
                for (size_t m = 1; m < (DELIMITERS_PER_SPLIT - l); ++m) {
                  if (new_delimiter_values[cur_pos] ==
                      new_delimiter_values[cur_pos + m]) {
                    last = rel_pos + m;
                  } else {
                    break;
                  }
                }
                break;
              } else {
                cur_pos++;
                rel_pos++;
              }
            }
            if (rel_pos != DELIMITERS_PER_SPLIT) {
              if (last > first) {
                rel_pos = first + (rand() % (last-first));
              } else {
                rel_pos = first;
              }
            }
          } else {
            for (size_t l = 0; l < DELIMITERS_PER_SPLIT; ++l) {
              if (data_objects[j][new_delimiter_dimensions[k]] <=
                  new_delimiter_values[cur_pos]) {
                break;
              } else {
                rel_pos++;
                cur_pos++;
              }
            }
          }
          rel_positions.push_back(rel_pos);
          size_t times = 0;
          cur_pos = 0;
          // rel_positions is of size k (the variable)
          for (int m = (rel_positions.size() - 1); m >= 0; m--) {
            cur_pos += rel_positions[m] *
                       pow((DELIMITERS_PER_SPLIT+1), times++);
          }
          cur_pos = (this->getNumberOfNodesInTreeOfHeight(k+1) + cur_pos) *
                      DELIMITERS_PER_SPLIT;
        }
        cur_pos = (cur_pos - (this->getNumberOfNodesInTreeOfHeight(new_height)
                     * DELIMITERS_PER_SPLIT))
                  / DELIMITERS_PER_SPLIT;
        new_buckets[cur_pos]->InsertObject(data_objects[j], tids[j]);
      }
    } else { // super bucket
      for (size_t z = 0; z < SUPER_BUCKET_SIZE; ++z) {
        std::vector<std::vector<float> > &data_objects =
          ((BBTreeSuperBucket*) this->buckets[i])->GetDataObjects(z);
        std::vector<uint32_t> &tids =
          ((BBTreeSuperBucket*) this->buckets[i])->GetTids(z);
        for (size_t j = 0; j < data_objects.size(); ++j) {
          // determine new bucket loation
          size_t cur_pos = 0;
          std::vector<size_t> rel_positions;
          for (size_t k = 0; k < new_height; ++k) {
            size_t rel_pos = 0;
            if (k == (new_height - 1)) {
              int first = 0;
              int last = 0;
              for (size_t l = 0; l < DELIMITERS_PER_SPLIT; ++l) {
                if (data_objects[j][new_delimiter_dimensions[k]] <=
                    new_delimiter_values[cur_pos]) {
                  first = rel_pos;
                  last = rel_pos;
                  for (size_t m = 1; m < (DELIMITERS_PER_SPLIT - l); ++m) {
                    if (new_delimiter_values[cur_pos] ==
                        new_delimiter_values[cur_pos + m]) {
                      last = rel_pos + m;
                    } else {
                      break;
                    }
                  }
                  break;
                } else {
                  cur_pos++;
                  rel_pos++;
                }
              }
              if (rel_pos != DELIMITERS_PER_SPLIT) {
                if (last > first) {
                  rel_pos = first + (rand() % (last-first));
                } else {
                  rel_pos = first;
                }
              }
            } else {
              for (size_t l = 0; l < DELIMITERS_PER_SPLIT; ++l) {
                if (data_objects[j][new_delimiter_dimensions[k]] <=
                    new_delimiter_values[cur_pos]) {
                  break;
                } else {
                  rel_pos++;
                  cur_pos++;
                }
              }
            }
            rel_positions.push_back(rel_pos);
            size_t times = 0;
            cur_pos = 0;
            // rel_positions is of size k (the variable)
            for (int m = (rel_positions.size() - 1); m >= 0; m--) {
              cur_pos += rel_positions[m] *
                         pow((DELIMITERS_PER_SPLIT+1), times++);
            }
            cur_pos = (this->getNumberOfNodesInTreeOfHeight(k+1) + cur_pos) *
                        DELIMITERS_PER_SPLIT;
          }
          cur_pos = (cur_pos - (this->getNumberOfNodesInTreeOfHeight(new_height)
                       * DELIMITERS_PER_SPLIT))
                    / DELIMITERS_PER_SPLIT;
          new_buckets[cur_pos]->InsertObject(data_objects[j], tids[j]);
        }
      }
    }
    // decrease memory pressure
    // this should be disabled if RebuildDelimiters() is run in the background
    delete this->buckets[i];
  }

  // TODO: make the following lines atomic (necessary for background execution)
  delete this->delimiter_dimensions;
  delete this->delimiter_values;
  delete [] this->buckets;
  this->delimiter_dimensions = new_delimiter_dimensions;
  this->delimiter_values = new_delimiter_values;
  this->buckets = new_buckets;
  this->num_buckets = new_num_buckets;
  this->height = new_height;
  this->num_super_buckets = 0;
  this->num_empty_buckets = 0;
}
