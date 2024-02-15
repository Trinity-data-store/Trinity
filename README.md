# EuroSys '24 Artifact Evaluation for "Trinity: A Fast Compressed Multi-attribute Data Store"

## Hardware and OS

No special hardware requirement. We used node `xl170` on Ubuntu 20.04. 

Cloudlab Profile is attached in `scripts/cloudlab_profile.py`. Modify accordingly. 

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
