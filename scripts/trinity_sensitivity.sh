#!/bin/bash

cd build
make

echo "*** sensitivity_num_dimensions ***"
taskset -c 3 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -o SM -d 9 & 
sleep 10

taskset -c 14 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -o SM -d 8 & 
sleep 10

taskset -c 15 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -o SM -d 7 & 
sleep 10

taskset -c 16 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -o SM -d 6 & 
sleep 10

taskset -c 17 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -o SM -d 5 & 
sleep 10

taskset -c 18 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -o SM -d 4 & 
sleep 10

echo "*** sensitivity_selectivity ***"
taskset -c 4 /proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_selectivity -o SM
sleep 10
