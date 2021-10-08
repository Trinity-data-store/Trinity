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

Useful Link to debug:
https://stackoverflow.com/questions/44633043/cmake-libcurl-was-built-with-ssl-disabled-https-not-supported
https://stackoverflow.com/questions/56941778/cmake-use-system-curl-is-on-but-a-curl-is-not-found
https://thrift.apache.org/docs/BuildingFromSource
https://stackoverflow.com/questions/59561902/boost-thread-hpp-no-such-file-or-directory/59563726
https://stackoverflow.com/questions/21530577/fatal-error-python-h-no-such-file-or-directory?rq=1
If redefinition, remove build
repeat this process...