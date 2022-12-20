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
#define GITHUB_SIZE 200000000 // 200M
#define TPCH_DIMENSION 9
#define GITHUB_DIMENSION 10
#define TPCH_DATA_ADDR "/mntData2/tpch/data_300/tpch_processed_1B.csv"
#define GITHUB_DATA_ADDR "/mntData/github_events_processed_9.csv"
#define TPCH_QUERY_ADDR "/proj/trinity-PG0/Trinity/queries/tpch/tpch_query_converted"
#define GITHUB_QUERY_ADDR "/proj/trinity-PG0/Trinity/queries/github/github_query_new_converted"
#define QUERY_NUM 1000
// #define OPTIMIZATION_CN
// #define OPTIMIZATION_GM
    
level_t max_depth = 32;
level_t trie_depth = 6;
preorder_t max_tree_node = 512;
point_t points_to_insert = 30000;
point_t points_to_lookup = 30000;
std::string results_folder_addr = "/proj/trinity-PG0/Trinity/results/";
std::string identification_string = "";

/* [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE] */
std::vector<int32_t> tpch_max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 58063825, 19980802};
std::vector<int32_t> tpch_min_values = {1, 90000, 0, 0, 19920102, 19920131, 19920103, 81300, 19920101};

/* [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE] */
std::vector<int32_t> github_max_values = {7451541, 737170, 262926, 354850, 379379, 3097263, 703341, 8745, 20201206, 20201206};
std::vector<int32_t> github_min_values = {1, 1, 0, 0, 0, 0, 0, 0, 20110211, 20110211};

void use_github_setting(void) {

    std::vector<level_t> bit_widths = {24, 24, 24, 24, 24, 24, 24, 16, 24, 24}; // 10 Dimensions;
    std::vector<level_t> start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 10 Dimensions;
    total_points_count = GITHUB_SIZE;

    #ifdef OPTIMIZATION_B
    bit_widths = {24, 24, 24, 24, 24, 24, 24, 24, 24, 24};
    start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    identification_string = "B";
    total_points_count /= 5;
    #endif

    #ifdef OPTIMIZATION_CN
    bit_widths = {24, 24, 24, 24, 24, 24, 24, 24, 24, 24};
    start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    identification_string = "+CN";
    #endif

    #ifdef OPTIMIZATION_GM
    bit_widths = {24, 24, 24, 24, 24, 24, 24, 16, 24, 24}; 
    start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    identification_string = "+GM";
    #endif

    trie_depth = 6;
    max_depth = 24;
    no_dynamic_sizing = true;
    total_points_count = 200000000; 
    max_tree_node = 512;
    
    bitmap::CompactPtrVector tmp_ptr_vect(total_points_count);
    p_key_to_treeblock_compact = &tmp_ptr_vect;

    create_level_to_num_children(bit_widths, start_bits, max_depth);
}

void use_tpch_setting(int dimensions) {

    std::vector<level_t> bit_widths = {8, 32, 16, 24, 20, 20, 20, 32, 20}; // 9 Dimensions;
    std::vector<level_t> start_bits = {0, 0, 8, 16, 0, 0, 0, 0, 0}; // 9 Dimensions;
    total_points_count = TPCH_SIZE; 

    #ifdef OPTIMIZATION_B
    bit_widths = {32, 32, 32, 32, 32, 32, 32, 32, 32};
    start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    identification_string = "B";
    total_points_count /= 5;
    #endif

    #ifdef OPTIMIZATION_CN
    bit_widths = {32, 32, 32, 32, 32, 32, 32, 32, 32};
    start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    identification_string = "+CN";
    #endif

    #ifdef OPTIMIZATION_GM
    bit_widths = {8, 32, 16, 24, 20, 20, 20, 32, 20};
    start_bits = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    identification_string = "+GM";
    #endif

    start_bits.resize(dimensions);
    bit_widths.resize(dimensions);

    trie_depth = 6;
    max_depth = 32;
    no_dynamic_sizing = true;
    bitmap::CompactPtrVector tmp_ptr_vect(total_points_count);
    p_key_to_treeblock_compact = &tmp_ptr_vect;
    create_level_to_num_children(bit_widths, start_bits, max_depth);
}

void flush_vector_to_file(std::vector<TimeStamp> vect, std::string filename){

    std::ofstream outFile(filename);
    for (const auto &e : vect) outFile << std::to_string(e) << "\n";
}


void get_query_github(std::string line, data_point<GITHUB_DIMENSION> *start_range, data_point<GITHUB_DIMENSION> *end_range) {


    for (dimension_t i = 0; i < GITHUB_DIMENSION; i++){
        start_range->set_coordinate(i, github_min_values[i]);
        end_range->set_coordinate(i, github_max_values[i]);
    }

    std::stringstream ss(line);

    while (ss.good()) {

        std::string index_str;
        std::getline(ss, index_str, ',');

        std::string start_range_str;
        std::getline(ss, start_range_str, ',');
        std::string end_range_str;
        std::getline(ss, end_range_str, ',');

        // std::cout << start_range_str << " " << end_range_str << std::endl;
        int index = std::stoul(index_str);
        if (index > 10)
            index -= 3;

        if (start_range_str != "-1") {
            start_range->set_coordinate(index, std::stoul(start_range_str));
        }
        if (end_range_str != "-1") {
            end_range->set_coordinate(index, std::stoul(end_range_str));
        }
    }
    
    for (dimension_t i = 0; i < GITHUB_DIMENSION; i++){
        if (i >= 8) {
            start_range->set_coordinate(i, start_range->get_coordinate(i) - 20110000);
            end_range->set_coordinate(i, end_range->get_coordinate(i) - 20110000);            
        }
    }
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

int gen_rand(int start, int end) {
    return start + ( std::rand() % ( end - start + 1 ) );
}

#endif //TrinityCommon_H
