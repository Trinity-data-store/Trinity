# EuroSys '24 Artifact Evaluation for "Trinity: A Fast Compressed Multi-attribute Data Store"

## Hardware and OS

No special hardware requirement. We used node `xl170` on Ubuntu 20.04. 

An example Cloudlab Profile is attached in `scripts/cloudlab_profile.py`. 

## Setup

```bash
bash scripts/setup_one_node.sh
```

To run a distributed setup, execute the setup scripts on all client and storage nodes. 

## Build

```bash
mkdir -p build
cd build
cmake -DGENERATE_THRIFT=on -Dbuild_rpc=on ..
make
```

## Datasets


## Citation
```bibtex

``` 

## The MIT License
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)  
