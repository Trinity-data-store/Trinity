#include "trie.h"
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <vector>
#include <iostream>
#include <fstream>
#include <string> 

const dimension_t DIMENSION = 7;

void run_bench(level_t max_depth, level_t trie_depth, preorder_t max_tree_node, bool run_preset_query = false, bool run_search_query = false, bool load_from_File = false){

    // raise(SIGINT);
    std::vector<int32_t> found_points;
    std::vector<data_point<DIMENSION>> all_points;

    md_trie<DIMENSION> mdtrie(max_depth, trie_depth, max_tree_node);

    data_point<DIMENSION> leaf_point;

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    FILE *fp = fopen("../data/fs/fs_dataset.txt", "r");

    // If the file cannot be open
    if (fp == nullptr)
    {
        fprintf(stderr, "file not found\n");
        exit(EXIT_FAILURE);
    }
    
    n_leaves_t n_points = 0;
    uint64_t max[DIMENSION];
    uint64_t min[DIMENSION];

    TimeStamp start, diff;
    diff = 0;

    /**
     * Insertion
     */

    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *token = strtok(line, " ");
        char *ptr;

        for (uint8_t i = 1; i <= 2; i ++){
            token = strtok(nullptr, " ");
        }

        for (dimension_t i = 0; i < DIMENSION; i++){
            token = strtok(nullptr, " ");
            leaf_point.set_coordinate(i, strtoull(token, &ptr, 10) % std::numeric_limits<uint32_t>::max());
        }

        for (dimension_t i = 0; i < DIMENSION; i++){
            
            if (n_points == 0){
                max[i] = leaf_point.get_coordinate(i);
                min[i] = leaf_point.get_coordinate(i);
            }
            else {
                if (leaf_point.get_coordinate(i) > max[i]){
                    max[i] = leaf_point.get_coordinate(i);
                }
                if (leaf_point.get_coordinate(i) < min[i]){
                    min[i] = leaf_point.get_coordinate(i);
                }
            }       
        }

        start = GetTimestamp();
        if (!load_from_File) {
            mdtrie.insert_trie(&leaf_point, n_points, p_key_to_treeblock_compact);
            diff += GetTimestamp() - start;
        }
        n_points ++;
        all_points.push_back(leaf_point);
        if (n_points == total_points_count)
            break;
    }

    std::cout << "Insertion Latency: " << (float) diff / total_points_count << std::endl;
    std::cout << "mdtrie storage: " << mdtrie.size(p_key_to_treeblock_compact) << std::endl;
    fclose(fp);

    if (!load_from_File) {

        std::filebuf fb;

        fb.open ("mdtrie_fs.txt",std::ios::out);
        std::ostream os(&fb);    
        size_t serialized_size = mdtrie.Serialize(os);
        std::cout << "Serialized size: " << serialized_size << std::endl;
        std::cout << "Current file offset: " << current_file_offset << std::endl;
        fb.close();
        raise(SIGINT);

        current_file_offset = 0;
        fb.open ("mdtrie_fs.txt",std::ios::out); // Intend to overwrite
        std::ostream os_new(&fb);    
        serialized_size = mdtrie.Serialize(os_new, true);  
        std::cout << "Serialized size: " << serialized_size << std::endl;
        std::cout << "Current file offset: " << current_file_offset << std::endl;
        fb.close();
        exit(0);

        // Just write to file
        if (!run_preset_query && !run_search_query)
            exit(0);
    }

    if (load_from_File) {

        std::filebuf fb_in;
        fb_in.open ("mdtrie_fs.txt",std::ios::in);
        std::istream is(&fb_in);
        size_t in_size;
        in_size = mdtrie.Deserialize(is, true);  // Use offset
        std::cout << "deserilize in_size: " << in_size << std::endl;
        fb_in.close();
        
        for (unsigned int i = 0; i < all_points.size(); i ++) {
            if (!mdtrie.check(&all_points[i])) {
                std::cout << "Check failed!\n"; 
                exit(0);
            }
        }
    }

    /**
     * Benchmark range search given a query selectivity (1000-2000), given a query
     */

    data_point<DIMENSION> start_range;
    data_point<DIMENSION> end_range;
    
    if (run_preset_query){

        line = nullptr;
        len = 0;
        fp = fopen("../queries/fs/fs_range_queries.csv", "r");
        read = getline(&line, &len, fp);
        diff = 0;
        int count = 0;
        std::vector<TimeStamp> latency_vect;

        while ((read = getline(&line, &len, fp)) != -1)
        {
            char *ptr;
            char *token = strtok(line, ",");

            for (dimension_t i = 0; i < DIMENSION - 1; i++){
                token = strtok(nullptr, ","); // start
                start_range.set_coordinate(i, strtoul(token, &ptr, 10));
                token = strtok(nullptr, ","); // end
                end_range.set_coordinate(i, strtoul(token, &ptr, 10));
            }
            if (DIMENSION > 6){
                start_range.set_coordinate(6, min[6]);
                end_range.set_coordinate(6, max[6]);
            }

            std::vector<int32_t> found_points_temp;
            start = GetTimestamp();
            mdtrie.range_search_trie(&start_range, &end_range, mdtrie.root(), 0, found_points_temp);
            TimeStamp temp_diff =  GetTimestamp() - start; 
            latency_vect.push_back(temp_diff);
            diff += temp_diff;
            count ++;
            found_points_temp.clear();
        }
        std::cout << "Average query latency: " << (float) diff / count << std::endl;    
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

        std::cout << "Find range query started, created range_search_filesystem.csv" << std::endl;
        int itr = 0;
        std::ofstream file("../queries/fs/range_search_filesystem_sensitivity_" + std::to_string(discount_factor) + ".csv", std::ios_base::app);

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

    for (n_leaves_t i = 0; i < found_points_size; i++){

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

    preorder_t treeblock_size = 512;
    uint32_t trie_depth = 10;
    level_t max_depth = 32;
    discount_factor = 1;
    // if (argc == 2){
    //     discount_factor = atoi(argv[1]);
    // }
    total_points_count = 14583357 / discount_factor;
    no_dynamic_sizing = false;

    std::cout << "dimension: " << DIMENSION << std::endl;
    std::cout << "trie depth: " << trie_depth << std::endl;
    std::cout << "treeblock sizes: " << treeblock_size << std::endl;
    std::cout << "discount factor: " << discount_factor << std::endl;
    std::cout << "total_points_count: " << total_points_count << std::endl;

    bitmap::CompactPtrVector tmp_ptr_vect(total_points_count);
    p_key_to_treeblock_compact = &tmp_ptr_vect;

    std::vector<level_t> bit_widths = {32, 32, 32, 32, 24, 24, 32}; // 7 Dimensions    
    std::vector<level_t> start_bits = {0, 0, 0, 0, 0, 0, 0}; // 7 Dimensions    

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
        if (atoi(argv[1]) == 2) {
            std::cout << "load mdtrie from file" << std::endl;
            run_bench(max_depth, trie_depth, treeblock_size, true, false, true);
        }
        else if (atoi(argv[1]) == 1) {
            run_bench(max_depth, trie_depth, treeblock_size, false, true);
        }
        else if (atoi(argv[1]) == 3) {  // Serialize
            run_bench(max_depth, trie_depth, treeblock_size, false, false, false);
        }
        else {
            std::cerr << "wrong command!" << std::endl;
            exit(0);
        }
    }
    else 
        run_bench(max_depth, trie_depth, treeblock_size);
}