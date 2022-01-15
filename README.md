# Trinity

This repository is the official implementation of [Trinity: A Fast and Space-efficient Multi-attribute Data Store]().  

## Install Dependencies

```
cd ..
source setup.sh
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

## Reproduce Results

```
nohup sh scripts/microbench.sh &
nohup sh scripts/dataset_size_sensitivity.sh &
nohup sh scripts/rstar_microbench.sh &
```

## Contributing

### The MIT License
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)  