# md-trie

### Building

```bash
mkdir -p build
cd build
cmake -DGENERATE_THRIFT=on ..
make
```

### Datasets

**File System (FS) Dataset**, a private dataset of metadata collected from a distributed file system. 

**OpenStreetMap (OSM) Dataset**, geographical records catalogued by the OpenStreetMap project. [Link](https://download.geofabrik.de/)   
To extract the data, use Python Osmium package and a sample script is provided. [Here](data/OSM/process_osm.py)  

**TPC-H Dataset**, a synthetic business dataset. We coalesce its lineitem and orders tables. [Link](https://docs.deistercloud.com/content/Databases.30/TPCH%20Benchmark.90/Data%20generation%20tool.30.xml?embedded=true/)

Put the dataset under /libmdtrie/bench/data

### Tests

```bash
make test
```

### Benchmark

Remember to set the number of dimensions, bit widths, and starting level of each attribute.
The benchmark file can be found in libmdtrie/bench

### File Structure

n the "md-trie/libmdtrie/src" folder,   
trie.h defines the main function calls for the data structure, such as insert_trie and range_search_trie.  
tree_block.h defines the function calls for each treeblock, including insert, child and range_search_treeblock, select_subtree selects a node and its subtree to turn into a frontier node.  
This implementation includes a top-level pointer-based trie data structure, where each trie node, defined in trie_node.h, stores an array of pointers to children trie nodes. The size of this top-level trie could be adjusted. Its leaves point to child treeblocks.  
compressed_bitmap.h defines the structure of the bit vector in the treeblocks and supports collapsed nodes.  
compact_ptr.h implements how primary keys are stored at the leaf, either stored directly as value, in a vector, or a delta-encoded array (defined in delta_encoded_array.h)  

### Debug

Install dependencies...
```
sudo apt-get install libboost-test-dev  
sudo apt-get install libboost-all-dev
sudo apt-get install curl
sudo apt-get install libssl-dev libcurl4-openssl-dev
sudo apt install libboost-thread-dev
sudo apt-get install python3-dev  # for python3.x installs
wget https://github.com/Kitware/CMake/releases/download/v3.22.0/cmake-3.22.0.tar.gz
tar -xf cmake-3.22.0.tar.gz
cd cmake-3.22.0/
./bootstrap --system-curl
make
sudo make install
cd ..
wget https://dlcdn.apache.org/thrift/0.15.0/thrift-0.15.0.tar.gz
tar -xf thrift-0.15.0.tar.gz
cd thrift-0.15.0/
./bootstrap.sh
./configure
make
sudo make install
```

https://stackoverflow.com/questions/44633043/cmake-libcurl-was-built-with-ssl-disabled-https-not-supported  
https://stackoverflow.com/questions/56941778/cmake-use-system-curl-is-on-but-a-curl-is-not-found  
https://thrift.apache.org/docs/BuildingFromSource  
https://stackoverflow.com/questions/59561902/boost-thread-hpp-no-such-file-or-directory/59563726  
https://stackoverflow.com/questions/21530577/fatal-error-python-h-no-such-file-or-directory?rq=1  
https://blog.csdn.net/m0_51560548/article/details/121574598  
If redefinition bug, remove build
```  
sudo apt-get install libboost-test-dev  
sudo apt-get install libboost-all-dev
```  
reinstall everything and re-run 
```
Thrift Install  
```
