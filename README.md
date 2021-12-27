# Trinity

This repository is the official implementation of [Trinity: A Fast and Space-efficient Multi-attribute Data Store]().  

## Install Dependencies

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

## Build

```setup
mkdir -p build
cd build
cmake -DGENERATE_THRIFT=on ..
make
```

## Run Unit Tests
    make test

## Simple Example
A simple example can be found [here](libmdtrie/bench/tpch_bench.cpp). 

### File Structure

trie.h defines the main function calls for the data structure, such as insert_trie and range_search_trie.  
tree_block.h defines the function calls for each treeblock, including insert, child and range_search_treeblock, select_subtree selects a node and its subtree to turn into a frontier node.  
This implementation includes a top-level pointer-based trie data structure, where each trie node, defined in trie_node.h, stores an array of pointers to children trie nodes. The size of this top-level trie could be adjusted. Its leaves point to child treeblocks.  
compressed_bitmap.h defines the structure of the bit vector in the treeblocks and supports collapsed nodes.  
compact_ptr.h implements how primary keys are stored at the leaf, either stored directly as value, in a vector, or a delta-encoded array (defined in delta_encoded_array.h)  

## Contributing

### The MIT License
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)  