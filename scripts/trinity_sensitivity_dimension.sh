#!/bin/bash

cd build
make

/proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -d 4
/proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -d 8
/proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -d 16
/proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -d 32
/proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -d 64
/proj/trinity-PG0/Trinity/build/libmdtrie/microbench -b sensitivity_num_dimensions -d 128
