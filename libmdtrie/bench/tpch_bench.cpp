#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <tqdm.h>
#include <vector>
#include <iostream>
#include <fstream>


// const int DATA_DIMENSION = 8;
level_t TRIE_DEPTH = 10;
uint32_t TREEBLOCK_SIZE = 1024;
std::ofstream myfile;

void run_bench(level_t max_depth, level_t trie_depth, preorder_t max_tree_node, std::vector<level_t> dimension_bits){
    
    create_level_to_num_children(dimension_bits, max_depth);
    auto *found_points = new point_array();
    auto *all_points = new std::vector<data_point>;

    auto *mdtrie = new md_trie(max_depth, trie_depth, max_tree_node);
    auto *leaf_point = new data_point();

    std::ifstream infile("/home/ziming/tpch-dbgen/data/orders_lineitem_merged.csv");

    std::string line;
    std::getline(infile, line);
    std::vector<uint32_t> max_values(DATA_DIMENSION, 0);
    std::vector<uint32_t> min_values(DATA_DIMENSION, 4294967295);

    n_leaves_t n_points = 0;
    n_leaves_t n_lines = total_points_count;

    // n_lines = 60000000;

    tqdm bar;
    TimeStamp start, diff;
    diff = 0;

    while (std::getline(infile, line))
    {
        bar.progress(n_points, n_lines);
        std::stringstream ss(line);
        std::vector<std::string> string_result;

        // Parse string by ","
        int leaf_point_index = 0;
        int index = -1;
        while (ss.good())
        {
            index ++;
            std::string substr;
            std::getline(ss, substr, ',');
        
            uint32_t num;
            if (index == 5 || index == 6 || index == 7 || index == 16) // float with 2dp
            {
                num = static_cast<uint32_t>(std::stof(substr) * 100);
            }
            else if (index == 10 || index == 11 || index == 12 || index == 17) //yy-mm-dd
            {
                substr.erase(std::remove(substr.begin(), substr.end(), '-'), substr.end());
                num = static_cast<uint32_t>(std::stoul(substr));
            }
            else if (index == 8 || index == 9 || index == 13 || index == 15 || index == 18) //skip text
                continue;
            else if (index == 0 || index == 1 || index == 2 || index == 14) // secondary keys
                continue;
            else if (index == 19) // all 0
                continue;
            else
                num = static_cast<uint32_t>(std::stoul(substr));

            leaf_point->set_coordinate(leaf_point_index, num);
            leaf_point_index++;
        }
        

        for (dimension_t i = 0; i < DATA_DIMENSION; i++){
            if (leaf_point->get_coordinate(i) > max_values[i])
                max_values[i] = leaf_point->get_coordinate(i);
            if (leaf_point->get_coordinate(i) < min_values[i])
                min_values[i] = leaf_point->get_coordinate(i);         
        }

        (*all_points).push_back((*leaf_point));

        if (n_points == n_lines)
            break;

        start = GetTimestamp();
        mdtrie->insert_trie(leaf_point, n_points);
        diff += GetTimestamp() - start;

        n_points ++;

    }
    bar.finish();

    myfile << "Insertion Latency: " << (float) diff / n_lines << std::endl;
    myfile << "mdtrie storage: " << mdtrie->size() << std::endl;
    myfile << "trie_size: " << trie_size << std::endl;
    myfile << "p_key_to_treeblock_compact_size: " << p_key_to_treeblock_compact_size << std::endl;
    myfile << "total_treeblock_num: " << total_treeblock_num << std::endl;
    myfile << "treeblock_primary_pointer_size: " << treeblock_primary_pointer_size << std::endl;
    myfile << "treeblock_primary_size: " << treeblock_primary_size << std::endl;
    myfile << "treeblock_nodes_size: " << treeblock_nodes_size << std::endl;
    myfile << "collapsed_node_num: " << collapsed_node_num << std::endl;

    tqdm bar2;
    TimeStamp check_diff = 0;
    
    for (uint64_t i = 0; i < n_points; i++){
        bar2.progress(i, n_points);
        auto check_point = (*all_points)[i];
        start = GetTimestamp();

        if (!mdtrie->check(&check_point)){
            raise(SIGINT);
        } 
        check_diff += GetTimestamp() - start;  

    }

    bar2.finish();
    myfile << "Average time to check one point: " << (float) check_diff / n_lines << std::endl;


    auto *start_range = new data_point();
    auto *end_range = new data_point();

    for (dimension_t i = 0; i < DATA_DIMENSION; i++){
        start_range->set_coordinate(i, min_values[i]);
        end_range->set_coordinate(i, max_values[i]);
    }

    start = GetTimestamp();
    mdtrie->range_search_trie(start_range, end_range, mdtrie->root(), 0, found_points);
    diff = GetTimestamp() - start;

    myfile << "found_pts size: " << found_points->size() << std::endl;
    myfile << "Range Search Latency: " << (float) diff / found_points->size() << std::endl;


    n_leaves_t found_points_size = found_points->size();
    TimeStamp diff_primary = 0;

    n_leaves_t checked_points_size = 0;
    tqdm bar4;
    for (n_leaves_t i = 0; i < found_points_size; i += 5){
        checked_points_size++;

        bar4.progress(i, found_points_size);
        data_point *point = found_points->at(i);
        n_leaves_t returned_primary_key = point->read_primary();

        symbol_t *node_path_from_primary = (symbol_t *)malloc((max_depth + 1) * sizeof(symbol_t));

        tree_block *t_ptr = (tree_block *) (p_key_to_treeblock_compact.At(returned_primary_key));
        
        start = GetTimestamp();

        symbol_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(returned_primary_key, node_path_from_primary);
        node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;

        data_point *returned_coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary, DATA_DIMENSION);

        diff_primary += GetTimestamp() - start;

        for (dimension_t j = 0; j < DATA_DIMENSION; j++){
            if (returned_coordinates->get_coordinate(j) != point->get_coordinate(j)){
                raise(SIGINT);
            }
        }    

        auto correct_point = (* all_points)[returned_primary_key];
        for (dimension_t j = 0; j < DATA_DIMENSION; j++){
            if (returned_coordinates->get_coordinate(j) != correct_point.get_coordinate(j)){
                raise(SIGINT);
            }
        }          

        free(node_path_from_primary);
    }
    bar4.finish();
    myfile << "Lookup Latency: " << (float) diff_primary / checked_points_size << std::endl;    


}

void print_dimension_bits(std::vector<level_t>  dimension_bits, std::vector<level_t> new_start_dimension_bits)
{
    start_dimension_bits = new_start_dimension_bits;

    myfile << std::endl << "end_dimension_bits: ";
    for (uint8_t i = 0; i < dimension_bits.size(); i++){
        myfile << dimension_bits[i] << ", ";
    }

    myfile << std::endl << "start_dimension_bits: ";
    for (uint8_t i = 0; i < new_start_dimension_bits.size(); i++){
        myfile << new_start_dimension_bits[i] << ", ";
    }
    
    myfile << std::endl;    
}

int main() {

    TREEBLOCK_SIZE = 512;
    TRIE_DEPTH = 6;
    is_osm = true;

    myfile.open("tpch_benchmark_" + std::to_string(DATA_DIMENSION) + "_" + std::to_string(TRIE_DEPTH) + "_" + std::to_string(TREEBLOCK_SIZE) + ".txt", std::ios_base::app);
    std::vector<level_t> dimension_bits;
    std::vector<level_t> new_start_dimension_bits;


    myfile << "dimension: " << DATA_DIMENSION << std::endl;
    myfile << "trie depth: " << TRIE_DEPTH << std::endl;
    myfile << "treeblock sizes: " << TREEBLOCK_SIZE << std::endl;


    dimension_bits = {8, 16, 32, 24, 32, 32, 32, 32, 32, 32}; // 10 Dimensions
    new_start_dimension_bits = {0, 8, 0, 16, 24, 0, 0, 0, 0, 0}; // 10 Dimensions
    print_dimension_bits(dimension_bits, new_start_dimension_bits);

    run_bench(32, TRIE_DEPTH, TREEBLOCK_SIZE, dimension_bits);
    myfile << std::endl;


    
// *****************************************
    
    dimension_bits = {8, 16, 32, 24, 32, 64, 64, 32, 64, 32}; // 10 Dimensions
    new_start_dimension_bits = {0, 8, 0, 16, 24, 32, 32, 0, 32, 0}; // 10 Dimensions
    print_dimension_bits(dimension_bits, new_start_dimension_bits);

    run_bench(64, TRIE_DEPTH, TREEBLOCK_SIZE, dimension_bits);
    myfile << std::endl;
    exit(0);

// *****************************************
    TREEBLOCK_SIZE = 1024;
    myfile << "dimension: " << DATA_DIMENSION << std::endl;
    myfile << "trie depth: " << TRIE_DEPTH << std::endl;
    myfile << "treeblock sizes: " << TREEBLOCK_SIZE << std::endl;

    dimension_bits = {8, 16, 32, 24, 32, 64, 64, 32, 64, 32}; // 10 Dimensions
    new_start_dimension_bits = {0, 8, 0, 16, 24, 32, 32, 0, 32, 0}; // 10 Dimensions
    print_dimension_bits(dimension_bits, new_start_dimension_bits);

    run_bench(64, TRIE_DEPTH, TREEBLOCK_SIZE, dimension_bits);
    myfile << std::endl;

// *****************************************

    dimension_bits = {8, 16, 32, 24, 32, 32, 32, 32, 32, 32}; // 10 Dimensions
    new_start_dimension_bits = {0, 8, 0, 16, 24, 0, 0, 0, 0, 0}; // 10 Dimensions
    print_dimension_bits(dimension_bits, new_start_dimension_bits);

    run_bench(32, TRIE_DEPTH, TREEBLOCK_SIZE, dimension_bits);
    myfile << std::endl;

}