#!/bin/sh

cd /home/ziming/md-trie/build/
make
NUM_REPEATS=20

for i in $( seq 7 $NUM_REPEATS )
do
    echo "**** fs_bench $i's iteration started! ****" >> ../results_log/microbench_fs.log
    libmdtrie/fs_bench >> ../results_log/microbench_fs.log
    echo "\n" >> ../results_log/microbench_fs.log
done

for i in $( seq 18 $NUM_REPEATS )
do
    echo "**** osm_bench  $i's iteration started! ****" >> ../results_log/microbench_osm.log
    libmdtrie/osm_bench >> ../results_log/microbench_osm.log
    echo "\n" >> ../results_log/microbench_osm.log
done

for i in $( seq 18 $NUM_REPEATS )
do
    echo "**** tpch_bench  $i's iteration started! ****" >> ../results_log/microbench_tpch.log
    libmdtrie/tpch_bench >> ../results_log/microbench_tpch.log
    echo "\n" >> ../results_log/microbench_tpch.log
done
