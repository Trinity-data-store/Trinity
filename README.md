# EuroSys '24: "Trinity: A Fast Compressed Multi-attribute Data Store"

**Abstract**: With the proliferation of attribute-rich machine-generated data, emerging real-time monitoring, diagnosis, and visualization tools ingest and analyze such data across multiple attributes simultaneously. Due to the sheer volume of the data, applications need storage-efficient and performant data representations to analyze them efficiently.
  
We present Trinity, a system that simultaneously facilitates query and storage efficiency across large volumes of multi-attribute records. Trinity accomplishes this through a new dynamic, succinct, multi-dimensional data structure, MdTrie. MdTrie employs a combination of novel Morton code generalization, a multi-attribute query algorithm, and a self-indexed trie structure to achieve the above goals. 

## Folder Structure

- libmdtrie: The main code for MdTrie data structure.
  - trie.h: defines the main function calls for the data structure, such as insert_trie and range_search_trie.  
  - tree_block.h: defines the function calls for each treeblock, including insert, child and range_search_treeblock, select_subtree selects a node and its subtree to turn into a frontier node.    
  - trie_node.h: MdTrie in this implementation includes an optional top-level pointer-based trie data structure defined in trie_node.h. Its leaves point to child treeblocks.  
  - compressed_bitmap.h: defines the structure of the bit vector in the treeblocks and supports collapsed nodes.  
  - compact_ptr.h: implements how primary keys are stored at the leaf, either stored directly as value, in a vector, or a delta-encoded array (defined in delta_encoded_array.h)  
- librpc: Trinity implementation based on Thrift
- modules: Cmake dependencies for the code
- scripts: Scripts used to run the evaluations. 
- thrift: Thrift source files. 

## Hardware and OS

No special hardware requirement. We used node `xl170` on Ubuntu 20.04. 

An example Cloudlab Profile is attached in `scripts/cloudlab_profile.py`. 

## Setup

```bash
bash scripts/setup_one_node.sh
```

To run a distributed setup, execute the setup scripts on all client and storage nodes, or: 

```bash
bash scripts/setup_all_nodes.sh
```

## Build

```bash
mkdir -p build
cd build
cmake -DGENERATE_THRIFT=on -Dbuild_rpc=on ..
make
```

## Minimal Example

Check out the file `libmdtrie/bench/example.cpp` for example.
```c
  mdtrie.insert_trie(&point, primary_key);
  mdtrie.lookup_trie(primary_key);
  mdtrie.range_search_trie(&start_range, &end_range, found_points);
```
To start a local Trinity storage server

```bash
  librpc/MDTrieShardServer -i [IP] -s [NUM_SHARD] -d [DATASET]
```

An example Trinity client interface is provided `MDTrieShardClient.h`.

## Datasets and Queries

Datasets can be downloaded from GCS buckets. Since MdTrie and Trinity mainly index integer / floating point fields, we clean datasets to be integer fields. The raw dataset can be found online. 
- TPC-H [[Link](https://storage.cloud.google.com/trinity_datsets/tpch_dataset.csv)]
- Github Events [[Link](https://storage.googleapis.com/trinity_datsets/github_dataset.csv)]
- NYC [[Link](https://storage.googleapis.com/trinity_datsets/nyc_dataset.csv)]

Due to the lack of open-source multi-attribute range search queries, we generate 1k queries from query templates. You can find the queries in `queries` folder. 

## Contact

Ziming Mao (ziming.mao@berkeley.edu)

## License

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
