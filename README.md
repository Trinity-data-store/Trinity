# md-trie

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
https://thrift.apache.org/docs/BuildingFromSource.html
Remember to run the test suite and debug from there
https://stackoverflow.com/questions/59561902/boost-thread-hpp-no-such-file-or-directory/59563726