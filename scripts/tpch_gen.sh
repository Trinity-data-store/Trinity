#!/bin/sh

# tpch-gen
git clone https://github.com/electrum/tpch-dbgen.git
make -f makefile.suite
mkdir data
cd data
cp ../dbgen .
cp ../dists.dss .
./dbgen -s 5
