#!/bin/sh

cd /home/ziming/md-trie/build/
make
NUM_REPEATS=1

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** fs_bench $i's iteration started! ****" >> ../results_log/macrobench_fs.log
    librpc/TrinityFS >> ../results_log/macrobench_fs.log
    echo "\n" >> ../results_log/macrobench_fs.log
done

for i in $( seq 12 $NUM_REPEATS )
do
    echo "**** osm_bench  $i's iteration started! ****" >> ../results_log/macrobench_osm.log
    librpc/TrinityOSM >> ../results_log/macrobench_osm.log
    echo "\n" >> ../results_log/macrobench_osm.log
done

for i in $( seq 12 $NUM_REPEATS )
do
    echo "**** tpch_bench  $i's iteration started! ****" >> ../results_log/macrobench_tpch.log
    librpc/TrinityTPCH >> ../results_log/macrobench_tpch.log
    echo "\n" >> ../results_log/macrobench_tpch.log
done
