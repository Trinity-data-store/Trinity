#include "BBTreeBucket.h"

/**
 * BBTreeRegularBucket::IsRegularBucket() always returns true.
 */
bool BBTreeRegularBucket::IsRegularBucket() const {
  return true;
}

/**
 * BBTreeRegularBucket::IsFull(max_size) returns true if the bucket holds
 * more than max_size data objects, and false if not.
 */
bool BBTreeRegularBucket::IsFull(const size_t max_size) const {
  return (this->data_objects.size() >= max_size);
}

/**
 * BBTreeRegularBucket::GetObject(i) returns the i'th object of the
 * stored data objects.
 */
std::vector<float> BBTreeRegularBucket::GetObject(const size_t index) const {
  return this->data_objects[index];
}

/**
 * BBTreeRegularBucket::GetRandomObject() returns a random data objects.
 */
std::vector<float> BBTreeRegularBucket::GetRandomObject() const {
  return this->data_objects[rand() % this->count];
}

/**
 * BBTreeRegularBucket::GetTid(i) returns the tid of the
 * i'th stored data object.
 */
uint32_t BBTreeRegularBucket::GetTid(const size_t index) const {
  return this->tids[index];
}

/**
 * BBTreeRegularBucket::GetNumberOfObjects() returns the number of data objects
 * currently stored in the bucket.
 */
size_t BBTreeRegularBucket::GetNumberOfObjects() const {
  return this->data_objects.size();
}

/**
 * BBTreeRegularBucket::GetDataObjects() returns all stored data objects.
 * Beware: For performance reasons, it returns the reference, not a copy!
 */
std::vector<std::vector<float> >& BBTreeRegularBucket::GetDataObjects() {
  return this->data_objects;
}

/**
 * BBTreeRegularBucket::GetTids() returns all stored tids.
 * Beware: For performance reasons, it returns the reference, not a copy!
 */
std::vector<uint32_t>& BBTreeRegularBucket::GetTids() {
  return this->tids;
}

/**
 * BBTreeRegularBucket::InsertObject(feature_vector, tid) inserts the given
 * data object with the given tid.
 */
void BBTreeRegularBucket::InsertObject(const std::vector<float> feature_vector,
                                      const uint32_t object_id) {
  this->data_objects.push_back(feature_vector);
  this->tids.push_back(object_id);
  this->count++;
}

/**
 * BBTreeRegularBucket::BulkInsert(feature_vectors, tids) inserts the given
 * data objects with the given tids.
 */
void BBTreeRegularBucket::BulkInsert(const std::vector<std::vector<float> > &feature_vectors,
                                     const std::vector<uint32_t> &object_ids,
                                     const size_t start,
                                     const size_t end) {
  this->count = end - start;
  this->data_objects.insert(this->data_objects.end(),
                            feature_vectors.begin() + start,
                            feature_vectors.begin() + end);
  this->tids.insert(this->tids.end(),
                    object_ids.begin() + start,
                    object_ids.begin() + end);
}

/**
 * BBTreeRegularBucket::SearchObject(search_object) executes a point query.
 * It the given point query object is found, it returns its tid.
 * If no matching object is found, it returns -1.
 */
int32_t BBTreeRegularBucket::SearchObject(const std::vector<float> &search_object) const {
  bool match;

  for (size_t i = 0; i < this->count; ++i) {
    match = true;
    for (size_t j = 0; j < search_object.size(); ++j) {
      if (this->data_objects[i][j] != search_object[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      return this->tids[i];
    }
  }

  return -1;
}

/**
 * BBTreeRegularBucket::SearchRange(results,lower_bounds,upper_bounds) executes
 * a range query and stores the tids of all matching data objects in the given
 * std::vector results.
 */
void BBTreeRegularBucket::SearchRange(std::vector<uint32_t> &results,
                                      const std::vector<float> &lower_boundary,
                                      const std::vector<float> &upper_boundary) {
  bool match;

  for (size_t i = 0; i < this->count; ++i) {
    match = true;
    for (size_t j = 0; j < lower_boundary.size(); ++j) {
      if (this->data_objects[i][j] < lower_boundary[j] ||
          this->data_objects[i][j] > upper_boundary[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      results.push_back(this->tids[i]);
    }
  }
}

/**
 * BBTreeRegularBucket::DeleteObject(feature_vector) determines if the specified
 * feature vector exists in the bucket.
 * If it exists, it deletes it and returns true. Otherwise, it returns false.
 */
bool BBTreeRegularBucket::DeleteObject(const std::vector<float> feature_vector) {
  for (size_t i = 0; i < this->count; ++i) {
    bool match = true;
    for (size_t j = 0; j < feature_vector.size(); ++j) {
      if (this->data_objects[i][j] != feature_vector[j]) {
        match = false;
        break;
      }
    }
    if (match) { // data object has been found
      this->data_objects[i] = this->data_objects.back();
      this->tids[i] = this->tids.back();
      this->data_objects.pop_back();
      this->tids.pop_back();
      this->count--;
      return true;
    }
  }

  // data object has not been found
  return false;
}

/**
 * BBTreeSuperBucket::IsRegularBucket() always returns false.
 */
bool BBTreeSuperBucket::IsRegularBucket() const {
  return false;
}

/**
 * BBTreeSuperBucket::IsFull(max_size) returns true if the z
 * buckets hold more than max_size data objects in total,
 * and false if not.
 */
bool BBTreeSuperBucket::IsFull(const size_t max_size) const {
  return (this->count >= (this->num_buckets * max_size));
}

/**
 * BBTreeSuperBucket::GetObject(i) returns the i'th data object
 * of the first bucket.
 */
std::vector<float> BBTreeSuperBucket::GetObject(const size_t index) const {
  return this->data_objects[0][index];
}

/**
 * BBTreeSuperBucket::GetRandomObject() returns a data object
 * randomly chosen from the z buckets.
 */
std::vector<float> BBTreeSuperBucket::GetRandomObject() const {
  size_t bucket = rand() % this->num_buckets;
  while (this->data_objects[bucket].size() == 0) {
    bucket = rand() % this->num_buckets;
  }
  return this->data_objects[bucket][rand() % this->data_objects[bucket].size()];
}

/**
 * BBTreeSuperBucket::getTid(i) returns the tid of the i'th data object
 * of the first bucket.
 */
uint32_t BBTreeSuperBucket::GetTid(const size_t index) const {
  return this->tids[0][index];
}

/**
 * BBTreeSuperBucket::getNumberOfObjects() gets the total number of data objects
 * stored in all z buckets.
 */
size_t BBTreeSuperBucket::GetNumberOfObjects() const {
  return this->count;
}

/**
 * BBTreeSuperBucket::GetDataObjects(bucket_id) returns the data objects of the
 * bucket_id'th bucket.
 * Beware: For performance reasons it returns a reference, not a copy.
 */
std::vector<std::vector<float> >& BBTreeSuperBucket::GetDataObjects(const size_t bucket_id) {
  return this->data_objects[bucket_id];
}


/**
 * BBTreeSuperBucket::GetTids(bucket_id) returns the tids of the
 * bucket_id'th bucket.
 * Beware: For performance reasons it returns a reference, not a copy.
 */
std::vector<uint32_t>& BBTreeSuperBucket::GetTids(const size_t bucket_id) {
  return this->tids[bucket_id];
}

/**
 * BBTreeSuperBucket::InsertObject(feature_vector, object_id) inserts
 * the given feature vector with the given tid into the super bucket.
 * The actual bucket (of the z buckets) is chosen according to the delimiter
 * dimension and the z-1 delimiter values of the superbucket.
 */
void BBTreeSuperBucket::InsertObject(const std::vector<float> feature_vector,
                                     const uint32_t object_id) {
  const size_t bucket_id = this->getBucket(feature_vector);
  this->data_objects[bucket_id].push_back(feature_vector);
  this->tids[bucket_id].push_back(object_id);
  this->count++;
}

/**
 * BBTreeSuperBucket::BulkInsert(feature_vectors, tids) inserts the given
 * data objects with the given tids.
 */
void BBTreeSuperBucket::BulkInsert(const std::vector<std::vector<float> > &feature_vectors,
                                   const std::vector<uint32_t> &object_ids,
                                   const size_t start,
                                   const size_t end) {
  // TODO
}

/**
 * BBTreeSuperBucket::SearchObject(search_objects) executes the given point query.
 * If the point query object is found, it returns its tid. Otherwise,
 * it returns -1.
 */
int32_t BBTreeSuperBucket::SearchObject(const std::vector<float> &search_object) const {
  bool match;
  const size_t bucket_id = this->getBucket(search_object);

  for (size_t i = 0; i < this->data_objects[bucket_id].size(); ++i) {
    match = true;
    for (size_t j = 0; j < search_object.size(); ++j) {
      if (this->data_objects[bucket_id][i][j] != search_object[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      return this->tids[bucket_id][i];
    }
  }

  return -1;
}

/**
 * BBTreeSuperBucket::SearchRange(results,lower_bounds,upper_bounds) executes
 * the given range query and stores all matching data objects in the
 * std::vector results.
 * It exploits the delimiter dimension and values to execute the range query
 * only on the subset of relevant buckets.
 */
void BBTreeSuperBucket::SearchRange(std::vector<uint32_t> &results,
  								                  const std::vector<float> &lower_boundary,
                                    const std::vector<float> &upper_boundary) {
  bool match;
  std::vector<size_t> buckets;

  if (this->delimiter_values[0] >= lower_boundary[this->delimiter_dimension]) {
    buckets.push_back(0);
  }
  for (size_t i = 1; i < (this->num_buckets - 2); ++i) {
    if (this->delimiter_values[i] >= lower_boundary[this->delimiter_dimension] &&
        this->delimiter_values[i-1] <= upper_boundary[this->delimiter_dimension]) {
      buckets.push_back(i);
    }
  }
  if (this->delimiter_values[this->num_buckets - 2] <
      upper_boundary[this->delimiter_dimension]) {
    buckets.push_back(this->num_buckets - 1);
  }

  for (size_t i = 0; i < buckets.size(); ++i) {
    const size_t cur_bucket = buckets[i];
    for (size_t j = 0; j < this->data_objects[cur_bucket].size(); ++j) {
      match = true;
      for (size_t k = 0; k < lower_boundary.size(); ++k) {
        if (this->data_objects[cur_bucket][j][k] < lower_boundary[k] ||
            this->data_objects[cur_bucket][j][k] > upper_boundary[k]) {
          match = false;
          break;
        }
      }
      if (match) {
        results.push_back(this->tids[cur_bucket][j]);
      }
    }
  }
}


/**
 * BBTreeSuperBucket::DeleteObject(feature_vector) deletes the given
 * feature vector from the superbucket.
 * If the feature vector has been found and deleted, it returns true.
 * Otherwise, it returns false.
 */
bool BBTreeSuperBucket::DeleteObject(const std::vector<float> feature_vector) {
  const size_t bucket_id = this->getBucket(feature_vector);

  for (size_t i = 0; i < this->data_objects[bucket_id].size(); ++i) {
    bool match = true;
    for (size_t j = 0; j < feature_vector.size(); ++j) {
      if (this->data_objects[bucket_id][i][j] != feature_vector[j]) {
        match = false;
        break;
      }
    }
    if (match) { // feature vector has been found
      this->data_objects[bucket_id][i] = this->data_objects[bucket_id].back();
      this->tids[bucket_id][i] = this->tids[bucket_id].back();
      this->data_objects[bucket_id].pop_back();
      this->tids[bucket_id].pop_back();
      this->count--;
      return true;
    }
  }

  // feature vector has not been found
  return false;
}

/**
 * According to the delimiter dimension and values of the superbucket,
 * BBTreeSuperBucket::getBucket(feature_vector) returns the bucket
 * relevant for the given feature vector.
 */
inline size_t BBTreeSuperBucket::getBucket(const std::vector<float> &feature_vector) const {
  size_t bucket_id = 0;
  for (size_t i = 0; i < (this->num_buckets - 1); ++i) {
    if (feature_vector[this->delimiter_dimension] <= this->delimiter_values[i]) {
      break;
    } else {
      bucket_id++;
    }
  }

  return bucket_id;
}
