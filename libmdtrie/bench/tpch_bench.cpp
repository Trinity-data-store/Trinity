#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <iostream>
#include <fstream>

const dimension_t DIMENSION = 9;

void run_bench(level_t max_depth, level_t trie_depth, preorder_t max_tree_node, bool run_preset_query = true, bool run_search_query = false){

    std::vector<int32_t> found_points;
    md_trie<DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    data_point<DIMENSION> leaf_point;

    std::ifstream infile("../data/tpc-h/tpch_dataset.csv");

    std::string line;
    std::getline(infile, line);
    std::vector<uint32_t> max(DIMENSION, 0);
    std::vector<uint32_t> min(DIMENSION, 4294967295);

    n_leaves_t n_points = 0;
    total_points_count = total_points_count / discount_factor;

    TimeStamp start, diff;
    diff = 0;

    /**
     * Insertion
     */

    while (std::getline(infile, line))
    {
        std::stringstream ss(line);
        std::vector<std::string> string_result;

        int leaf_point_index = 0;
        int index = -1;

        //  Kept indexes: [4, 5, 6, 7, 10, 11, 12, 16, 17]
        //  [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
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
            else if (index == 3) // lineitem
                continue;
            else
                num = static_cast<uint32_t>(std::stoul(substr));

            leaf_point.set_coordinate(leaf_point_index, num);
            leaf_point_index++;
            if (leaf_point_index == DIMENSION)
                break;
        }
        
        for (dimension_t i = 0; i < DIMENSION; i++){
            if (leaf_point.get_coordinate(i) > max[i])
                max[i] = leaf_point.get_coordinate(i);
            if (leaf_point.get_coordinate(i) < min[i])
                min[i] = leaf_point.get_coordinate(i);         
        }

        if (n_points == total_points_count)
            break;

        start = GetTimestamp();
        mdtrie.insert_trie(&leaf_point, n_points);
        diff += GetTimestamp() - start;

        n_points ++;
        if (n_points % (total_points_count / 10) == 0)
            std::cout << "Inserted - n_points: " << n_points << std::endl;

        if (n_points == total_points_count)
            break;
    }

    std::cout << "Insertion Latency: " << (float) diff / total_points_count << std::endl;
    std::cout << "mdtrie storage: " << mdtrie.size() << std::endl;

    /**
     * Benchmark range search given a query selectivity (1000-2000), given a query
     */

    data_point<DIMENSION> start_range;
    data_point<DIMENSION> end_range;

    if (run_preset_query){

        char *line_c = nullptr;
        size_t len = 0;
        FILE *fp = fopen("../queries/tpch/tpch_range_queries.csv", "r");
        ssize_t read = getline(&line_c, &len, fp);
        diff = 0;
        int count = 0;
        std::vector<TimeStamp> latency_vect;

        while ((read = getline(&line_c, &len, fp)) != -1)
        {
            char *ptr;
            char *token = strtok(line_c, ",");

            for (dimension_t i = 0; i < DIMENSION; i++){
                token = strtok(nullptr, ","); // start
                start_range.set_coordinate(i, strtoul(token, &ptr, 10));
                token = strtok(nullptr, ","); // end
                end_range.set_coordinate(i, strtoul(token, &ptr, 10));
            }

            std::vector<int32_t> found_points_temp;
            start = GetTimestamp();
            mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points_temp);
            TimeStamp temp_diff =  GetTimestamp() - start; 
            diff += temp_diff;
            latency_vect.push_back(temp_diff);
            // if (found_points_temp.size() > 2000 || found_points_temp.size() < 1000)
                // std::cout << "found points size: " << found_points_temp.size() << ", index:  " << count << std::endl;
            count ++;
            found_points_temp.clear();
        }
        std::cout << "average query latency: " << (float) diff / count << std::endl; 
        TimeStamp squared_cumulative = 0;
        for (unsigned int i = 0; i < latency_vect.size(); i++){
            squared_cumulative += (latency_vect[i] - diff / count) * (latency_vect[i] - diff / count);
        }
        std::cout << "Standard Deviation: " << (float) sqrt (squared_cumulative / (count - 1)) << std::endl;
    }

   /**
     * Benchmark range search given a query selectivity (1000-2000), find new query
     */

    if (run_search_query){

        std::cout << "Find range query started, created range_search_tpch.csv" << std::endl;
        int itr = 0;
        std::ofstream file("../queries/tpch/range_search_tpch_sensitivity_" + std::to_string(discount_factor) + ".csv", std::ios_base::app);

        while (itr < 100){

            for (unsigned int j = 0; j < DIMENSION; j++){
                start_range.set_coordinate(j, min[j] + (max[j] - min[j] + 1) / 10 * (rand() % 10));
                end_range.set_coordinate(j, start_range.get_coordinate(j) + (max[j] - start_range.get_coordinate(j) + 1) / 10 * (rand() % 10));
            }

            std::vector<int32_t> found_points_temp;
            start = GetTimestamp();
            mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points_temp);
            diff = GetTimestamp() - start;

            if (found_points_temp.size() >= 1000 && found_points_temp.size() <= 2000){

                file << found_points_temp.size() << "," << diff << ",";
                for (unsigned int i = 0; i < DIMENSION; i++){
                    file << start_range.get_coordinate(i) << "," << end_range.get_coordinate(i) << ",";
                }
                file <<  std::endl;
                itr ++;
                if (itr % 10 == 0)
                    std::cout << "find query: " << itr << " done!" << std::endl;
            }
        }    
    }

    /**
     * Range Search with full range
     */

    for (dimension_t i = 0; i < DIMENSION; i++){
        start_range.set_coordinate(i, min[i]);
        end_range.set_coordinate(i, max[i]);
    }

    start = GetTimestamp();
    mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points);
    diff = GetTimestamp() - start;

    std::cout << "found_pts size: " << found_points.size() << std::endl;
    std::cout << "Full Range Search Latency per point: " << (float) diff / found_points.size() << std::endl;

    /**
     * Point lookup given primary keys returned by range search
     */

    n_leaves_t found_points_size = found_points.size();
    TimeStamp diff_primary = 0;
    n_leaves_t checked_points_size = 0;

    for (n_leaves_t i = 0; i < found_points_size; i += 5){

        n_leaves_t primary_key = found_points[i];
        std::vector<morton_t> node_path_from_primary(max_depth + 1);

        tree_block<DIMENSION> *t_ptr = (tree_block<DIMENSION> *) (p_key_to_treeblock_compact->At(primary_key));
        
        start = GetTimestamp();
        morton_t parent_symbol_from_primary = t_ptr->get_node_path_primary_key(primary_key, node_path_from_primary);
        node_path_from_primary[max_depth - 1] = parent_symbol_from_primary;
        data_point<DIMENSION> *coordinates = t_ptr->node_path_to_coordinates(node_path_from_primary, DIMENSION);
        diff_primary += GetTimestamp() - start;
        delete coordinates;
        checked_points_size++;
    }
    std::cout << "Lookup Latency: " << (float) diff_primary / checked_points_size << std::endl; 
}

int main(int argc, char *argv[]) {

    /**
     * Set hyperparameters
     * treeblock_size: maximum number of nodes a treeblock can hode
     * trie_depth: the maximum level of the top-level pointer-based trie structure
     * max_depth: the depth of whole data structure
     * bit_widths: the bit widths of each column, with default start-level all set to 0.
     */

    level_t trie_depth = 6;
    uint32_t treeblock_size = 512;
    discount_factor = 1;
    if (argc == 2){
        discount_factor = atoi(argv[1]);
    }
    total_points_count = 300005812 / discount_factor;
    no_dynamic_sizing = true;

    std::cout << "Data Dimension: " << DIMENSION << std::endl;
    std::cout << "trie depth: " << trie_depth << std::endl;
    std::cout << "treeblock sizes: " << treeblock_size << std::endl;
    std::cout << "discount factor: " << discount_factor << std::endl;
    std::cout << "total_points_count: " << total_points_count << std::endl;

    bitmap::CompactPtrVector tmp_ptr_vect(total_points_count);
    p_key_to_treeblock_compact = &tmp_ptr_vect;

    std::vector<level_t> bit_widths = {8, 32, 16, 24, 32, 32, 32, 32, 32}; // 9 Dimensions;
    std::vector<level_t> start_bits = {0, 0, 8, 16, 0, 0, 0, 0, 0}; // 9 Dimensions;

    level_t max_depth = 32;
    create_level_to_num_children(bit_widths, start_bits, max_depth);

    if (DIMENSION != bit_widths.size() || DIMENSION != start_bits.size()){
        std::cerr << "DATA DIMENSION does not match bit_widths vector!" << std::endl;
        exit(0);
    }

    if (total_points_count != p_key_to_treeblock_compact->get_num_elements()){
        std::cerr << "total_points_count does not match" << std::endl;
        exit(0);
    }
    if (argc == 2){
        run_bench(max_depth, trie_depth, treeblock_size, false, true);
    }
    else 
        run_bench(max_depth, trie_depth, treeblock_size);
}