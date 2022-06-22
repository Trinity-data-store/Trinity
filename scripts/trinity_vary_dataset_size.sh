#!/bin/sh

cd /home/ziming/md-trie/build/

NUM_REPEATS=1

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench $i's iteration for fraction 1/2 started! ****" >> ../results_log/dataset_size_sensitivity.log
    libmdtrie/tpch_bench 2 >> ../results_log/dataset_size_sensitivity.log
    echo "\n" >> ../results_log/dataset_size_sensitivity.log
done

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench $i's iteration for fraction 1/4 started! ****" >> ../results_log/dataset_size_sensitivity
    libmdtrie/tpch_bench 4 >> ../results_log/dataset_size_sensitivity.log
    echo "\n" >> ../results_log/dataset_size_sensitivity.log
done

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench $i's iteration for fraction 1/8 started! ****" >> ../results_log/dataset_size_sensitivity
    libmdtrie/tpch_bench 8 >> ../results_log/dataset_size_sensitivity.log
    echo "\n" >> ../results_log/dataset_size_sensitivity.log
done

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench $i's iteration for fraction 1/16 started! ****" >> ../results_log/dataset_size_sensitivity
    libmdtrie/tpch_bench 16 >> ../results_log/dataset_size_sensitivity
    echo "\n" >> ../results_log/dataset_size_sensitivity
done

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench $i's iteration for fraction 1/32 started! ****" >> ../results_log/dataset_size_sensitivity
    libmdtrie/tpch_bench 32 >> ../results_log/dataset_size_sensitivity
    echo "\n" >> ../results_log/dataset_size_sensitivity
done