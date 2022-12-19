#ifndef TrinityCommon_H
#define TrinityCommon_H

#include "trie.h"
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <fstream>

#define TPCH_SIZE 250000000 // 250M
#define TPCH_DIMENSION 9
#define TPCH_DATA_ADDR "/mntData2/tpch/data_300/tpch_processed_1B.csv"
#define TPCH_QUERY_ADDR "/proj/trinity-PG0/Trinity/queries/tpch/tpch_query_converted"
#define QUERY_NUM 1000


/**
 * Set hyperparameters
 * treeblock_size: maximum number of nodes a treeblock can hode
 * trie_depth: the maximum level of the top-level pointer-based trie structure
 * max_depth: the depth of whole data structure
 * bit_widths: the bit widths of each column, with default start-level all set to 0.
 */
    
level_t max_depth = 32;
level_t trie_depth = 6;
preorder_t max_tree_node = 512;
point_t points_to_insert = 30000;
point_t points_to_lookup = 30000;
std::string results_folder_addr = "/proj/trinity-PG0/Trinity/results/";


/* [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE] */
std::vector<int32_t> tpch_max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 58063825, 19980802};
std::vector<int32_t> tpch_min_values = {1, 90000, 0, 0, 19920102, 19920131, 19920103, 81300, 19920101};


void use_default_tpch_setting(void) {

    std::vector<level_t> bit_widths = {8, 32, 16, 24, 20, 20, 20, 32, 20};// 9 Dimensions;
    std::vector<level_t> start_bits = {0, 0, 8, 16, 0, 0, 0, 0, 0}; // 9 Dimensions;
    
    trie_depth = 6;
    max_depth = 32;
    no_dynamic_sizing = true;
    total_points_count = TPCH_SIZE; 
    bitmap::CompactPtrVector tmp_ptr_vect(total_points_count);
    p_key_to_treeblock_compact = &tmp_ptr_vect;
    create_level_to_num_children(bit_widths, start_bits, max_depth);
}

void flush_vector_to_file(std::vector<TimeStamp> vect, std::string filename){

    std::ofstream outFile(filename);
    for (const auto &e : vect) outFile << std::to_string(e) << "\n";
}

void get_query_tpch(std::string line, data_point<TPCH_DIMENSION> *start_range, data_point<TPCH_DIMENSION> *end_range) {


    for (dimension_t i = 0; i < TPCH_DIMENSION; i++){
        start_range->set_coordinate(i, tpch_min_values[i]);
        end_range->set_coordinate(i, tpch_max_values[i]);
    }

    std::stringstream ss(line);

    while (ss.good()) {

        std::string index_str;
        std::getline(ss, index_str, ',');

        std::string start_range_str;
        std::getline(ss, start_range_str, ',');
        std::string end_range_str;
        std::getline(ss, end_range_str, ',');

        if (start_range_str != "-1") {
            start_range->set_coordinate(std::stoul(index_str), std::stoul(start_range_str));
        }
        if (end_range_str != "-1") {
            end_range->set_coordinate(std::stoul(index_str), std::stoul(end_range_str));
        }
    }
    
    for (dimension_t i = 0; i < TPCH_DIMENSION; i++){
        if (i >= 4 && i != 7) {
            start_range->set_coordinate(i, start_range->get_coordinate(i) - 19000000);
            end_range->set_coordinate(i, end_range->get_coordinate(i) - 19000000);
        }
    }
}

#endif //TrinityCommon_H
