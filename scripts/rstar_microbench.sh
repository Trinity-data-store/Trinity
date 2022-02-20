#!/bin/sh

cd /home/ziming/md-trie/baselines/r-star-tree

g++ -o fs_out  fs_bench.cpp
g++ -o osm_out osm_bench.cpp
g++ -o tpch_out tpch_bench.cpp

NUM_REPEATS=2


# for i in $( seq 1 $NUM_REPEATS )
# for i in $( seq 1 $NUM_REPEATS )
# do
#     echo "**** osm_bench  $i's iteration started! ****" >> ../../results_log/microbench_osm_rstar.log
#     ./osm_out >> ../../results_log/microbench_osm_rstar.log
#     echo "\n" >> ../../results_log/microbench_osm_rstar.log
# done

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench  $i's iteration started! ****" >> ../../results_log/microbench_tpch_rstar.log
    ./tpch_out >> ../../results_log/microbench_tpch_rstar.log
    echo "\n" >> ../../results_log/microbench_tpch_rstar.log
done

# for i in $( seq 1 $NUM_REPEATS )
# do
#     echo "**** fs_bench $i's iteration started! ****" >> ../../results_log/microbench_fs_rstar.log
#     ./fs_out >> ../../results_log/microbench_fs_rstar.log
#     echo "\n" >> ../../results_log/microbench_fs_rstar.log
# done

