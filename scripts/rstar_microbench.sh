#!/bin/sh

cd /home/ziming/md-trie/baselines/r-star-tree

g++ -o fs_out  fs_bench.cpp
g++ -o osm_out osm_bench.cpp
g++ -o tpch_out tpch_bench.cpp

NUM_REPEATS=5

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** fs_bench $i's iteration started! ****" >> microbench_fs_rstar
    ./fs_out >> microbench_fs_rstar
    echo "\n" >> microbench_fs_rstar
done

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** osm_bench  $i's iteration started! ****" >> microbench_osm_rstar
    ./osm_out >> microbench_osm_rstar
    echo "\n" >> microbench_osm_rstar
done

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench  $i's iteration started! ****" >> microbench_tpch_rstar
    ./tpch_out >> microbench_tpch_rstar
    echo "\n" >> microbench_tpch_rstar
done
