#ifndef MdTrieBench_H
#define MdTrieBench_H

#include "common.hpp"
#include "trie.h"
#include <climits>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

template <dimension_t DIMENSION>
class MdTrieBench
{
public:
  MdTrieBench(md_trie<DIMENSION> *mdtrie) { mdtrie_ = mdtrie; };
  MdTrieBench(MdTries<DIMENSION> *mdtries) { mdtries_ = mdtries; }
  void insert(std::string data_addr,
              std::string outfile_name,
              n_leaves_t total_points_count,
              std::vector<int32_t> (*parse_line)(std::string line))
  {

    std::ifstream infile(data_addr);
    TimeStamp start = 0, diff = 0;
    n_leaves_t n_points = 0;
    n_leaves_t has_skipped = 0;
    data_point<DIMENSION> leaf_point;

    /**
     * Insertion
     */

    std::string line;
    while (std::getline(infile, line))
    {
      if (has_skipped < skip_size_count)
      {
        has_skipped++;
        continue;
      }

      std::vector<int32_t> vect = parse_line(line);
      for (dimension_t i = 0; i < DIMENSION; i++)
      {
        leaf_point.set_coordinate(i, vect[i % vect.size()]);
      }
      start = GetTimestamp();
      if (mdtrie_)
        mdtrie_->insert_trie(&leaf_point, n_points);
      else
        mdtries_->insert_trie(&leaf_point, n_points);
      TimeStamp latency = GetTimestamp() - start;
      diff += latency;
      n_points++;

      if (n_points > total_points_count - points_to_insert)
        insertion_latency_vect_.push_back(latency + SERVER_TO_SERVER_IN_NS);

      if (n_points == total_points_count)
        break;

      if (n_points % (total_points_count / 100) == 0)
        std::cout << n_points << " out of " << total_points_count << std::endl;
    }

    std::cout << "Insertion Latency: " << (float)diff / n_points << std::endl;
    flush_vector_to_file(insertion_latency_vect_,
                         results_folder_addr +
                             outfile_name);
    infile.close();
  }

  void lookup(std::string outfile_name)
  {

    TimeStamp cumulative = 0, start = 0;

    for (n_leaves_t i = 0; i < points_to_lookup; i++)
    {
      start = GetTimestamp();
      if (mdtrie_)
        mdtrie_->lookup_trie(i);
      else
        mdtries_->lookup_trie(i);
      TimeStamp temp_diff = GetTimestamp() - start;
      cumulative += temp_diff;
      lookup_latency_vect_.push_back(temp_diff + SERVER_TO_SERVER_IN_NS);
    }
    flush_vector_to_file(lookup_latency_vect_,
                         results_folder_addr +
                             outfile_name);
    std::cout << "Done! "
              << "Lookup Latency per point: "
              << (float)cumulative / points_to_lookup << std::endl;
  }

  void range_search(std::string query_addr,
                    std::string outfile_name,
                    void (*get_query)(std::string,
                                      data_point<DIMENSION> *,
                                      data_point<DIMENSION> *))
  {

    std::ifstream file(query_addr);
    std::ofstream outfile(results_folder_addr +
                          outfile_name);
    TimeStamp diff = 0, start = 0;

    for (int i = 0; i < QUERY_NUM; i++)
    {

      std::vector<int32_t> found_points;
      data_point<DIMENSION> start_range;
      data_point<DIMENSION> end_range;

      std::string line;
      std::getline(file, line);
      get_query(line, &start_range, &end_range);

      start = GetTimestamp();
      if (mdtrie_)
        mdtrie_->range_search_trie(
            &start_range, &end_range, found_points);
      else
        mdtries_->range_search_trie(
            &start_range, &end_range, found_points);
      diff = GetTimestamp() - start;
      outfile << "Query " << i << " end to end latency (ms): " << diff / 1000
              << ", found points count: " << found_points.size()
              << std::endl;
      found_points.clear();
    }
  }

  void range_search_random(std::string outfile_name,
                           void (*get_query)(data_point<DIMENSION> *,
                                             data_point<DIMENSION> *),
                           unsigned int upper_bound,
                           unsigned int lower_bound)
  {

    std::ofstream outfile(results_folder_addr +
                          outfile_name);
    TimeStamp diff = 0, start = 0;
    int i = 0;

    while (i < QUERY_NUM)
    {

      std::vector<int32_t> found_points;
      data_point<DIMENSION> start_range;
      data_point<DIMENSION> end_range;

      get_query(&start_range, &end_range);

      start = GetTimestamp();
      if (mdtrie_)
        mdtrie_->range_search_trie(
            &start_range, &end_range, found_points);
      else
        mdtries_->range_search_trie(
            &start_range, &end_range, found_points);
      diff = GetTimestamp() - start;

      if (found_points.size() > upper_bound ||
          found_points.size() < lower_bound)
        continue;

      outfile << "Query " << i << " end to end latency (ms): " << diff / 1000
              << ", found points count: " << found_points.size()
              << std::endl;
      found_points.clear();
      i += 1;
    }
  }

  void get_storage(std::string outfile_name)
  {

    uint64_t size;
    if (mdtrie_)
      size = mdtrie_->size();
    else
      size = mdtries_->size();
    std::cout << "mdtrie storage: " << size << std::endl;
    flush_string_to_file(
        std::to_string(size) + "," + std::to_string(total_points_count),
        results_folder_addr + outfile_name);
  }

protected:
  std::vector<TimeStamp> insertion_latency_vect_;
  std::vector<TimeStamp> lookup_latency_vect_;
  md_trie<DIMENSION> *mdtrie_ = NULL;
  MdTries<DIMENSION> *mdtries_ = NULL;
};

#endif // MdTrieBench_H
