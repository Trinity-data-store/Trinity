#!/bin/sh

cd /home/ziming/md-trie/baselines/phtree-cpp/build

make

NUM_REPEATS=20


for i in $( seq 5 $NUM_REPEATS )
do
    echo "**** osm_bench  $i's iteration started! ****" >> ../../../results_log/microbench_osm_phtree.log
    examples/Example 1 >> ../../../results_log/microbench_osm_phtree.log
    echo "\n" >> ../../../results_log/microbench_osm_phtree.log
done

for i in $( seq 5 $NUM_REPEATS )
do
    echo "**** tpch_bench  $i's iteration started! ****" >> ../../../results_log/microbench_tpch_phtree.log
    examples/Example 2 >> ../../../results_log/microbench_tpch_phtree.log
    echo "\n" >> ../../../results_log/microbench_tpch_phtree.log
done

for i in $( seq 5 $NUM_REPEATS )
do
    echo "**** fs_bench $i's iteration started! ****" >> ../../../results_log/microbench_fs_phtree.log
    examples/Example 0 >> ../../../results_log/microbench_fs_phtree.log
    echo "\n" >> ../../../results_log/microbench_fs_phtree.log
done

