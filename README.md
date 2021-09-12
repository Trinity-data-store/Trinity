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