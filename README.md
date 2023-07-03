# Trinity

Repository for [Trinity: A Fast and Space-efficient Multi-attribute Data Store]().   

## Install Dependencies
```
bash scripts/setup_one_node.sh
```

## Build

```setup
mkdir -p build
cd build
cmake -DGENERATE_THRIFT=on -Dbuild_rpc=on ..
make
```

### The MIT License
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)  
