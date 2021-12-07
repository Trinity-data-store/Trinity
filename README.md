# md-trie

Note: this repo is still under development. I have not yet cleaned it yet and written documentation.  
Some code might be outdated.  
For sample queries, please see: md-trie/libmdtrie/bench/osm_dataset_bench.cpp  
email ziming.mao@yale.edu if you have any questions.  

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
```bash
./build/libmdtrie/mdtrie_bench
```

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
