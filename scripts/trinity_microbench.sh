#!/bin/bash

cd build
make

echo "*** tpch ***"
taskset -c 0 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b tpch -o SM & 
sleep 10

echo "*** github ***"
taskset -c 1 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b github -o SM & 
sleep 10

echo "*** nyc ***"
taskset -c 2 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b nyc -o SM & 
sleep 10