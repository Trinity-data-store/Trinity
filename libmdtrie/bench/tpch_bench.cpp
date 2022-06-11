#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <iostream>
#include <fstream>

const dimension_t DIMENSION = 9;

// Parse one line from TPC-H file.
data_point<DIMENSION> parse_line_tpch(std::string line) {

    data_point<DIMENSION> point;
    int index = -1;
    bool primary_key = true;
    std::string delim = ",";
    auto start = 0U;
    auto end = line.find(delim);

    // [id, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
    while (end != std::string::npos)
    {
        std::string substr = line.substr(start, end - start); 
        start = end + 1;
        end = line.find(delim, start);

        if (primary_key) {
            primary_key = false;
            continue;
        }
        index ++;
        point.set_coordinate(index, static_cast<uint32_t>(std::stoul(substr)));
    }
    index ++; 
    std::string substr = line.substr(start, end - start); 
    point.set_coordinate(index, static_cast<uint32_t>(std::stoul(substr)));
    
    return point;
}

void run_bench_separate(level_t max_depth, level_t trie_depth, preorder_t max_tree_node){


    std::vector<int32_t> found_points;
    md_trie<DIMENSION/2> mdtrie_first(max_depth, trie_depth, max_tree_node);
    md_trie<DIMENSION - DIMENSION/2> mdtrie_second(max_depth, trie_depth, max_tree_node);

    data_point<DIMENSION> leaf_point;
    data_point<DIMENSION/2> leaf_point_first;
    data_point<DIMENSION - DIMENSION/2> leaf_point_second;

    std::ifstream infile("/mntData/tpch_split/x0");

    std::string line;
    std::getline(infile, line);
    std::vector<uint32_t> max(DIMENSION, 0);
    std::vector<uint32_t> min(DIMENSION, 4294967295);

    n_leaves_t n_points = 0;
    // total_points_count = total_points_count / discount_factor;
    total_points_count = total_points_count / discount_factor;

    TimeStamp start, diff;
    diff = 0;

    /**
     * Insertion
     */

    // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]

    while (std::getline(infile, line))
    {
        leaf_point = parse_line_tpch(line);
        
        for (dimension_t i = 0; i < DIMENSION; i++){
            /*
            if (i >= 4 && i != 7) {
                leaf_point.set_coordinate(i, leaf_point.get_coordinate(i) - 19000000);
            }
            */
            if (leaf_point.get_coordinate(i) > max[i]) {
                max[i] = leaf_point.get_coordinate(i);
            }
            if (leaf_point.get_coordinate(i) < min[i]) {
                min[i] = leaf_point.get_coordinate(i);  
            }       

            if (i < DIMENSION / 2) {
                leaf_point_first.set_coordinate(i, leaf_point.get_coordinate(i));
            } else {
                leaf_point_second.set_coordinate(i - DIMENSION/2, leaf_point.get_coordinate(i));
            }
        }

        if (n_points == total_points_count)
            break;

        start = GetTimestamp();
        std::vector<level_t>  bit_widths = {8, 32, 16, 24};
        std::vector<level_t>  start_bits = {0, 0, 8, 16};
        create_level_to_num_children(bit_widths, start_bits, max_depth);

        mdtrie_first.insert_trie(&leaf_point_first, n_points, p_key_to_treeblock_compact);
        bit_widths = {32, 32, 32, 32, 32};
        start_bits = {0, 0, 0, 0, 0};
        create_level_to_num_children(bit_widths, start_bits, max_depth);

        mdtrie_second.insert_trie(&leaf_point_second, n_points, p_key_to_treeblock_compact);
        diff += GetTimestamp() - start;

        n_points ++;
        if (n_points % (total_points_count / 10) == 0)
            std::cout << "Inserted - n_points: " << n_points << std::endl;

        if (n_points == total_points_count)
            break;
    }

    std::cout << "Insertion Latency: " << (float) diff / total_points_count << std::endl;


    // std::cout << "mdtrie storage: " << mdtrie_first.size(p_key_to_treeblock_compact) << std::endl;
    infile.close();

  
    data_point<DIMENSION> start_range;
    data_point<DIMENSION> end_range;

    data_point<DIMENSION/2> start_range_first;
    data_point<DIMENSION/2> end_range_first;
    data_point<DIMENSION - DIMENSION/2> start_range_second;
    data_point<DIMENSION - DIMENSION/2> end_range_second;

    /** 
        Range Search
    */

    // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
    std::vector<int32_t> max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 59591284, 19980802};
    std::vector<int32_t> min_values = {1, 90001, 0, 0, 19920102, 19920131, 19920103, 81602, 19920101};

    char *infile_address = (char *)"../baselines/clickhouse/long_query_test_for_trinity";
    
    std::ifstream file(infile_address);

    for (int i = 0; i < 4; i ++) {

        std::vector<int32_t> found_points;
        for (dimension_t i = 0; i < DIMENSION; i++){
            start_range.set_coordinate(i, min[i]);
            end_range.set_coordinate(i, max[i]);
        }

        std::string line;
        std::getline(file, line);

        std::stringstream ss(line);
        std::cout << line << std::endl;
      // Example: 0,-1,24,2,5,7,4,19943347,19950101
        while (ss.good()) {

            std::string index_str;
            std::getline(ss, index_str, ',');

            std::string start_range_str;
            std::getline(ss, start_range_str, ',');
            std::string end_range_str;
            std::getline(ss, end_range_str, ',');

            std::cout << start_range_str << " " << end_range_str << std::endl;
            if (start_range_str != "-1") {
                start_range.set_coordinate(std::stoul(index_str), std::stoul(start_range_str));
            }
            if (end_range_str != "-1") {
                end_range.set_coordinate(std::stoul(index_str), std::stoul(end_range_str));
            }
        }
        // raise(SIGINT);
        for (dimension_t i = 0; i < DIMENSION; i++) {
            if (i < DIMENSION / 2) {
                start_range_first.set_coordinate(i, start_range.get_coordinate(i));
                end_range_first.set_coordinate(i, end_range.get_coordinate(i));
            } else {
                start_range_second.set_coordinate(i - DIMENSION/2, start_range.get_coordinate(i));
                end_range_second.set_coordinate(i - DIMENSION/2, end_range.get_coordinate(i));
            }
        }

        std::cout << "Query " << i << " started" << std::endl;
        start = GetTimestamp();

        /*
        for (dimension_t i = 0; i < DIMENSION; i++){
            if (i >= 4 && i != 7) {
                start_range.set_coordinate(i, start_range.get_coordinate(i) - 19000000);
                end_range.set_coordinate(i, end_range.get_coordinate(i) - 19000000);
            }
        }
        */
        std::vector<level_t>  bit_widths = {8, 32, 16, 24};
        std::vector<level_t>  start_bits = {0, 0, 8, 16};
        create_level_to_num_children(bit_widths, start_bits, max_depth);
        mdtrie_first.range_search_trie(&start_range_first, &end_range_first, mdtrie_first.root(), 0, found_points);

        bit_widths = {32, 32, 32, 32, 32};
        start_bits = {0, 0, 0, 0, 0};
        create_level_to_num_children(bit_widths, start_bits, max_depth);
        mdtrie_second.range_search_trie(&start_range_second, &end_range_second, mdtrie_second.root(), 0, found_points);

        diff = GetTimestamp() - start;
        std::cout << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / DIMENSION << std::endl;
        found_points.clear();

        /*
        std::cout << "range search elapsed time: %ld ms" << (GetTimestamp() - start) / 1000 << std::endl;
        std::cout << "function_call_count: %ld" << function_call_count << std::endl;
        std::cout << "high_num_children: %ld" << high_num_children << std::endl;  
        std::cout << "update_start_end_range_time: %ld" << update_start_end_range_time / 1000 << std::endl;
        std::cout << "range_search_child_time: %ld" << range_search_child_time / 1000 << std::endl;
        std::cout << "update_symbol_time: %ld" << update_symbol_time / 1000 << std::endl;
        std::cout << "next_child_time: " << next_child_time / 1000 << std::endl;
        std::cout << "symbol_diff_count: " << symbol_diff_count << std::endl;
        std::cout << "not_going_down_count: " << not_going_down_count << std::endl; 
        */

        /*
        function_call_count = 0;
        high_num_children = 0;
        update_start_end_range_time = 0;
        range_search_child_time = 0;
        update_symbol_time = 0;
        next_child_time = 0;
        symbol_diff_count = 0;
        not_going_down_count = 0;

        for (int i = 0; i < 33; i++) {
            std::cout << branching_count[i] << ",";
            branching_count[i] = 0;
        }
        std::cout << std::endl;
        */
    }
    return;
}

void run_bench(level_t max_depth, level_t trie_depth, preorder_t max_tree_node, bool run_preset_query = true, bool run_search_query = false, bool load_from_File = false){

    std::vector<int32_t> found_points;
    md_trie<DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);
    data_point<DIMENSION> leaf_point;

    std::ifstream infile("/mntData/tpch_split/x0");

    std::string line;
    std::getline(infile, line);
    std::vector<uint32_t> max(DIMENSION, 0);
    std::vector<uint32_t> min(DIMENSION, 4294967295);

    n_leaves_t n_points = 0;
    // total_points_count = total_points_count / discount_factor;
    total_points_count = total_points_count / discount_factor;

    TimeStamp start, diff;
    diff = 0;

    /**
     * Insertion
     */

    // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]

    while (std::getline(infile, line))
    {
        leaf_point = parse_line_tpch(line);
        
        for (dimension_t i = 0; i < DIMENSION; i++){
            
            if (i >= 4 && i != 7) {
                leaf_point.set_coordinate(i, leaf_point.get_coordinate(i) - 19000000);
            }
            
            if (leaf_point.get_coordinate(i) > max[i]) {
                max[i] = leaf_point.get_coordinate(i);
            }
            if (leaf_point.get_coordinate(i) < min[i]) {
                min[i] = leaf_point.get_coordinate(i);  
            }       

        }

        if (n_points == total_points_count)
            break;

        start = GetTimestamp();
        if (!load_from_File)
            mdtrie.insert_trie(&leaf_point, n_points, p_key_to_treeblock_compact);
        diff += GetTimestamp() - start;

        n_points ++;
        if (n_points % (total_points_count / 10) == 0)
            std::cout << "Inserted - n_points: " << n_points << std::endl;

        if (n_points == total_points_count)
            break;
    }

    std::cout << "Insertion Latency: " << (float) diff / total_points_count << std::endl;
    std::cout << "mdtrie storage: " << mdtrie.size(p_key_to_treeblock_compact) << std::endl;
    infile.close();
    // usleep(15 * 1000000);

    /*
    if (!load_from_File) {

        std::filebuf fb;
        fb.open ("mdtrie_tpch.txt",std::ios::out);
        std::ostream os(&fb);    
        size_t serialized_size = mdtrie.Serialize(os);
        std::cout << "Serialized size: " << serialized_size << std::endl;
        fb.close();

        // Just write to file
        if (!run_preset_query && !run_search_query)
            exit(0);
    }

    if (load_from_File) {
        std::filebuf fb_in;
        fb_in.open ("mdtrie_tpch.txt",std::ios::in);
        std::istream is(&fb_in);
        size_t in_size;
        in_size = mdtrie.Deserialize(is);
        std::cout << "deserilize in_size: " << in_size << std::endl;
        fb_in.close();
    }
    */

  
    data_point<DIMENSION> start_range;
    data_point<DIMENSION> end_range;


    /** 
        Range Search
    */

    // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
    std::vector<int32_t> max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 59591284, 19980802};
    std::vector<int32_t> min_values = {1, 90001, 0, 0, 19920102, 19920131, 19920103, 81602, 19920101};

    char *infile_address = (char *)"../baselines/clickhouse/long_query_test_for_trinity";
    
    std::ifstream file(infile_address);

    for (int i = 0; i < 4; i ++) {

        std::vector<int32_t> found_points;
        for (dimension_t i = 0; i < DIMENSION; i++){
            start_range.set_coordinate(i, min[i]);
            end_range.set_coordinate(i, max[i]);
        }

        std::string line;
        std::getline(file, line);

        std::stringstream ss(line);
        std::cout << line << std::endl;
      // Example: 0,-1,24,2,5,7,4,19943347,19950101
        while (ss.good()) {

            std::string index_str;
            std::getline(ss, index_str, ',');

            std::string start_range_str;
            std::getline(ss, start_range_str, ',');
            std::string end_range_str;
            std::getline(ss, end_range_str, ',');

            std::cout << start_range_str << " " << end_range_str << std::endl;
            if (start_range_str != "-1") {
                start_range.set_coordinate(std::stoul(index_str), std::stoul(start_range_str));
            }
            if (end_range_str != "-1") {
                end_range.set_coordinate(std::stoul(index_str), std::stoul(end_range_str));
            }
        }
        std::cout << "Query " << i << " started" << std::endl;
        start = GetTimestamp();

        
        for (dimension_t i = 0; i < DIMENSION; i++){
            if (i >= 4 && i != 7) {
                start_range.set_coordinate(i, start_range.get_coordinate(i) - 19000000);
                end_range.set_coordinate(i, end_range.get_coordinate(i) - 19000000);
            }
        }
        

        mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points);
        diff = GetTimestamp() - start;
        std::cout << "Query " << i << " end to end latency (ms): " << diff / 1000 << ", found points count: " << found_points.size() / DIMENSION << std::endl;
        found_points.clear();

        /*
        std::cout << "range search elapsed time: %ld ms" << (GetTimestamp() - start) / 1000 << std::endl;
        std::cout << "function_call_count: %ld" << function_call_count << std::endl;
        std::cout << "high_num_children: %ld" << high_num_children << std::endl;  
        std::cout << "update_start_end_range_time: %ld" << update_start_end_range_time / 1000 << std::endl;
        std::cout << "range_search_child_time: %ld" << range_search_child_time / 1000 << std::endl;
        std::cout << "update_symbol_time: %ld" << update_symbol_time / 1000 << std::endl;
        std::cout << "next_child_time: " << next_child_time / 1000 << std::endl;
        std::cout << "symbol_diff_count: " << symbol_diff_count << std::endl;
        std::cout << "not_going_down_count: " << not_going_down_count << std::endl; 
        */

        /*
        function_call_count = 0;
        high_num_children = 0;
        update_start_end_range_time = 0;
        range_search_child_time = 0;
        update_symbol_time = 0;
        next_child_time = 0;
        symbol_diff_count = 0;
        not_going_down_count = 0;
        for (int i = 0; i < 33; i++) {
            std::cout << branching_count[i] << ",";
            branching_count[i] = 0;
        }
        std::cout << std::endl;
        */
    }
    return;

    /**
     * Range Search with full range
     */


    for (dimension_t i = 0; i < DIMENSION; i++){
        start_range.set_coordinate(i, min[i]);
        end_range.set_coordinate(i, max[i]);
    }

    // 0,17,19,2,0,10,4,19920102,19981201,5,19927913,19966622,6,19968479,19972109,8,19920101,19980802

    start_range.set_coordinate(0, 17);
    end_range.set_coordinate(0, 19);
    start_range.set_coordinate(2, 0);
    end_range.set_coordinate(2, 10);
    start_range.set_coordinate(4, 19920102);
    end_range.set_coordinate(4, 19981201);
    start_range.set_coordinate(5, 19927913);
    end_range.set_coordinate(5, 19966622);
    start_range.set_coordinate(6, 19968479);
    end_range.set_coordinate(6, 19972109);
    start_range.set_coordinate(8, 19920101);
    end_range.set_coordinate(8, 19980802);

    // raise(SIGINT);
    REUSE_RANGE_SEARCH_CHILD = true;
    start = GetTimestamp();
    mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points);
    diff = GetTimestamp() - start;

    std::cout << "found_pts size: " << found_points.size() / DIMENSION << std::endl;
    std::cout << "total_points_count: " << total_points_count << std::endl;
    // std::cout << "primary_key_vect_total: " << primary_key_vect_total.size() << std::endl;
    std::cout << "Full Range Search Latency per point: " << (float) diff / (found_points.size() / DIMENSION) << std::endl;

    found_points.clear();
    // raise(SIGINT);
    REUSE_RANGE_SEARCH_CHILD = false;
    start = GetTimestamp();
    mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points);
    diff = GetTimestamp() - start;

    std::cout << "found_pts size: " << found_points.size() / DIMENSION << std::endl;
    std::cout << "total_points_count: " << total_points_count << std::endl;
    // std::cout << "primary_key_vect_total: " << primary_key_vect_total.size() << std::endl;
    std::cout << "Full Range Search Latency per point: " << (float) diff / (found_points.size() / DIMENSION) << std::endl;

    return;

    /**
     * Benchmark range search given a query selectivity (1000-2000), given a query
     */


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
            std::cout << temp_diff << std::endl;
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

            if (found_points_temp.size() >= 1000 * DIMENSION && found_points_temp.size() <= 2000 * DIMENSION){

                file << found_points_temp.size() / DIMENSION << "," << diff << ",";
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
     * Point lookup given primary keys returned by range search
     */
    /*
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
    */
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
    discount_factor = 10;
    // if (argc == 2){
    //     discount_factor = atoi(argv[1]);
    // }
    total_points_count = 50000471;
    no_dynamic_sizing = true;

    std::cout << "Data Dimension: " << DIMENSION << std::endl;
    std::cout << "trie depth: " << trie_depth << std::endl;
    std::cout << "treeblock sizes: " << treeblock_size << std::endl;
    std::cout << "discount factor: " << discount_factor << std::endl;
    std::cout << "total_points_count: " << total_points_count << std::endl;

    bitmap::CompactPtrVector tmp_ptr_vect(total_points_count);
    p_key_to_treeblock_compact = &tmp_ptr_vect;

    // [QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
    // std::vector<int32_t> max_values = {50, 10494950, 10, 8, 19981201, 19981031, 19981231, 59591284, 19980802};
    // std::vector<int32_t> min_values = {1, 90001, 0, 0, 19920102, 19920131, 19920103, 81602, 19920101};
    level_t max_depth = 32;

    std::vector<level_t> bit_widths = {8, 32, 16, 24, 32, 32, 32, 32, 32}; // 9 Dimensions;
    std::vector<level_t> start_bits = {0, 0, 8, 16, 0, 0, 0, 0, 0}; // 9 Dimensions;

    bit_widths = {8, 32, 16, 24, 24, 24, 24, 32, 24};
    bit_widths = {8, 32, 16, 24, 20, 20, 20, 32, 20};

    create_level_to_num_children(bit_widths, start_bits, max_depth);
    for (level_t i = 0; i < 32; i++){
        std::cout << level_to_num_children[i] << " ";
    }
    std::cout << std::endl;
    /*
    if (DIMENSION != bit_widths.size() || DIMENSION != start_bits.size()){
        std::cerr << "DATA DIMENSION does not match bit_widths vector!" << std::endl;
        exit(0);
    }

    if (total_points_count != p_key_to_treeblock_compact->get_num_elements()){
        std::cerr << "total_points_count does not match" << std::endl;
        exit(0);
    }
    */
    if (argc == 2){
        if (atoi(argv[1]) == 2) {
            std::cout << "load mdtrie from file" << std::endl;
            run_bench(max_depth, trie_depth, treeblock_size, true, false, true);
        }
        else if (atoi(argv[1]) == 1) {
            std::cout << "run search query" << std::endl;
            run_bench(max_depth, trie_depth, treeblock_size, false, true);
        }
        else if (atoi(argv[1]) == 3) {
            run_bench(max_depth, trie_depth, treeblock_size, false, false, false);
        }
        else {
            std::cerr << "wrong command!" << std::endl;
            exit(0);
        }
    }
    else {
        run_bench(max_depth, trie_depth, treeblock_size);
        // run_bench_separate(max_depth, trie_depth, treeblock_size);
    }
}