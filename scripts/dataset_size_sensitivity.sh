#!/bin/sh

cd /home/ziming/md-trie/build/

NUM_REPEATS=1

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench $i's iteration for fraction 1/2 started! ****" >> dataset_size_sensitivity
    libmdtrie/tpch_bench 2 >> dataset_size_sensitivity
    echo "\n" >> dataset_size_sensitivity
done

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench $i's iteration for fraction 1/4 started! ****" >> dataset_size_sensitivity
    libmdtrie/tpch_bench 4 >> dataset_size_sensitivity
    echo "\n" >> dataset_size_sensitivity
done

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench $i's iteration for fraction 1/8 started! ****" >> dataset_size_sensitivity
    libmdtrie/tpch_bench 8 >> dataset_size_sensitivity
    echo "\n" >> dataset_size_sensitivity
done

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench $i's iteration for fraction 1/16 started! ****" >> dataset_size_sensitivity
    libmdtrie/tpch_bench 16 >> dataset_size_sensitivity
    echo "\n" >> dataset_size_sensitivity
done

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench $i's iteration for fraction 1/32 started! ****" >> dataset_size_sensitivity
    libmdtrie/tpch_bench 32 >> dataset_size_sensitivity
    echo "\n" >> dataset_size_sensitivity
done