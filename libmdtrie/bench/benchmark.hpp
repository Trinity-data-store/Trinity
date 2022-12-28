#ifndef MdTrieBench_H
#define MdTrieBench_H

#include "trie.h"
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <fstream>
#include "common.hpp"

template<dimension_t DIMENSION>
class MdTrieBench {
public:
    MdTrieBench(md_trie<DIMENSION> *mdtrie) {
        mdtrie_ = mdtrie;
    };

    void insert(std::string data_addr, std::string outfile_name, point_t total_points_count, std::vector<int32_t> (* parse_line)(std::string line)) {

        std::ifstream infile(data_addr);
        TimeStamp start = 0, diff = 0;
        point_t n_points = 0;
        point_t has_skipped = 0;
        data_point<DIMENSION> leaf_point;

        /**
         * Insertion
         */

        std::string line;
        while (std::getline(infile, line))
        {
            if (has_skipped < skip_size_count) {
                has_skipped ++;
                continue;
            }

            std::vector<int32_t> vect = parse_line(line);
            for (dimension_t i = 0; i < DIMENSION; i++) {
                leaf_point.set_coordinate(i, vect[i]);
            }
            start = GetTimestamp();
            mdtrie_->insert_trie(&leaf_point, n_points, p_key_to_treeblock_compact);
            TimeStamp latency =  GetTimestamp() - start;
            diff += latency;
            n_points ++;

            if (n_points > total_points_count - points_to_insert)
                insertion_latency_vect_.push_back(latency);

            if (n_points == total_points_count)
                break; 

            if (n_points % (total_points_count / 100) == 0)
                std::cout << n_points << " out of " << total_points_count << std::endl;
        }    

        std::cout << "Insertion Latency: " << (float) diff / n_points << std::endl;
        flush_vector_to_file(insertion_latency_vect_, results_folder_addr + "microbenchmark/" + outfile_name);
        infile.close();
    }

    void lookup(std::string outfile_name) {

        TimeStamp cumulative = 0, start = 0;

        for (point_t i = 0; i < points_to_lookup; i ++) {

            start = GetTimestamp();
            std::vector<morton_t> node_path_from_primary(max_depth + 1);
            tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *) (p_key_to_treeblock_compact->At(i));
            morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(i, node_path_from_primary);
            node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;
            t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);
            TimeStamp temp_diff = GetTimestamp() - start;
            cumulative += temp_diff;

            lookup_latency_vect_.push_back(temp_diff);
        }
        flush_vector_to_file(lookup_latency_vect_, results_folder_addr + "microbenchmark/" + outfile_name);
        std::cout << "Done! " << "Lookup Latency per point: " << (float) cumulative / points_to_lookup << std::endl;
    }

    void range_search(std::string query_addr, std::string outfile_name, void (*get_query) (std::string, data_point<DIMENSION> *, data_point<DIMENSION> *)) {

        std::ifstream file(query_addr);
        std::ofstream outfile(results_folder_addr + "microbenchmark/" + outfile_name);
        TimeStamp diff = 0, start = 0;

        for (int i = 0; i < QUERY_NUM; i ++) {

            std::vector<int32_t> found_points;
            data_point<DIMENSION> start_range;
            data_point<DIMENSION> end_range;

            std::string line;
            std::getline(file, line);
            get_query(line, &start_range, &end_range);

            start = GetTimestamp();
            mdtrie_->range_search_trie(&start_range, &end_range, mdtrie_->root(), 0, found_points);
            diff = GetTimestamp() - start;
            outfile << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / DIMENSION << std::endl;
            found_points.clear();
        }
    }

    void range_search_random(std::string outfile_name, void (*get_query) (data_point<DIMENSION> *, data_point<DIMENSION> *), unsigned int upper_bound, unsigned int lower_bound) {

        std::ofstream outfile(results_folder_addr + "microbenchmark/" + outfile_name);
        TimeStamp diff = 0, start = 0;
        int i = 0;

        while (i < QUERY_NUM * 10) {

            std::vector<int32_t> found_points;
            data_point<DIMENSION> start_range;
            data_point<DIMENSION> end_range;

            get_query(&start_range, &end_range);

            start = GetTimestamp();
            mdtrie_->range_search_trie(&start_range, &end_range, mdtrie_->root(), 0, found_points);
            diff = GetTimestamp() - start;

            if (found_points.size() / DIMENSION > upper_bound || found_points.size() / DIMENSION < lower_bound)
                continue;

            outfile << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / DIMENSION << std::endl;
            found_points.clear();
            i += 1;
        }
    }

    void get_storage(std::string outfile_name) {

        uint64_t size = mdtrie_->size(p_key_to_treeblock_compact);
        std::cout << "mdtrie storage: " << size << std::endl;
        flush_string_to_file(std::to_string(size) + "," + std::to_string(total_points_count), results_folder_addr + "microbenchmark/" + outfile_name);
    }

protected:
    std::vector<TimeStamp> insertion_latency_vect_;
    std::vector<TimeStamp> lookup_latency_vect_;
    md_trie<DIMENSION> *mdtrie_;
};

#endif //MdTrieBench_H

