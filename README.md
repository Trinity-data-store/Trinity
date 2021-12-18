# md-trie

### Datasets

File System (FS) Dataset, a private dataset comprising representative metadata collected from a distributed file system at a large enterprise storage vendor. This dataset contains $14$ million data points, each with 7 attributes.   

OpenStreetMap (OSM) Dataset, comprising 153 million records corresponding to the North America region catalogued by the OpenStreetMap project. [Link](https://download.geofabrik.de/)   
To extract the data, use Python Osmium package and a sample script is provided [here](data/OSM/process_osm.py)  

TPC-H Dataset, a synthetic business dataset comprising of 300 million points. We coalesce its lineitem and orders tables on the orderkey attribute, resulting in a single table with 300 million records, each containing 9 attributes. [Link](https://docs.deistercloud.com/content/Databases.30/TPCH%20Benchmark.90/Data%20generation%20tool.30.xml?embedded=true/) to download the OSM dataset, though you have to use Python osmium package to extract the data.  

### Building

```bash
mkdir build
cd build
cmake ..
make
```

### Tests

```bash
make test
```

### Benchmark

Put the dataset under /libmdtrie/bench/data  
Note, before running, check the def.h, remember to set the number of dimension,
In benchmark file, you will find: 
```
    std::vector<level_t> dimension_bits = {8, 32, 32, 32}; // 4 Dimensions
    std::vector<level_t> new_start_dimension_bits = {0, 0, 0, 0}; // 4 Dimensions
```
use create_level_to_num_children with these two vectors to set up.    
The first vector sets the bit widths along each dimension. The second vector sets the level that a dimension first becomes "active".  

### Thrift

```cd build
cmake -DGENERATE_THRIFT=on ..
```

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
