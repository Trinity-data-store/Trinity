#!/bin/bash

cd build
make

echo "*** sensitivity_optimizations ***"
taskset -c 5 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b tpch -o B &
sleep 10

taskset -c 6 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b tpch -o CN &
sleep 10

taskset -c 7 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b tpch -o GM &
sleep 10

taskset -c 8 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b github -o B &
sleep 10

taskset -c 9 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b github -o CN &
sleep 10

taskset -c 10 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b github -o GM &
sleep 10

taskset -c 11 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b nyc -o B &
sleep 10

taskset -c 12 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b nyc -o CN &
sleep 10

taskset -c 13 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b nyc -o GM &
sleep 10
