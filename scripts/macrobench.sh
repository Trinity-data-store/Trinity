#!/bin/sh

cd /home/ziming/md-trie/build/
make
NUM_REPEATS=2

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** fs_bench $i's iteration started! ****" >> ../results_log/macrobench_fs.log
    librpc/TrinityFS >> ../results_log/macrobench_fs.log
    printf "\n" >> ../results_log/macrobench_fs.log
    sleep 1s
done

# for i in $( seq 1 $NUM_REPEATS )
# do
#     echo "**** osm_bench  $i's iteration started! ****" >> ../results_log/macrobench_osm.log
#     librpc/TrinityOSM >> ../results_log/macrobench_osm.log
#     printf "\n" >> ../results_log/macrobench_osm.log
#     sleep 1s
# done

# for i in $( seq 2 $NUM_REPEATS )
# do
#     echo "**** tpch_bench  $i's iteration started! ****" >> ../results_log/macrobench_tpch_new.log
#     librpc/TrinityTPCH >> ../results_log/macrobench_tpch_new.log
#     printf "\n" >> ../results_log/macrobench_tpch_new.log
#     sleep 1s
# done
