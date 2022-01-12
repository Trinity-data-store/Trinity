#!/bin/sh

cd /home/ziming/md-trie/build/

NUM_REPEATS=1

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** fs_bench $i's iteration started! ****" >> microbench_fs
    libmdtrie/fs_bench >> microbench_fs
    echo "\n" >> microbench_fs
done

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** osm_bench  $i's iteration started! ****" >> microbench_osm
    libmdtrie/osm_bench >> microbench_osm
    echo "\n" >> microbench_osm
done

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench  $i's iteration started! ****" >> microbench_tpch
    libmdtrie/tpch_bench >> microbench_tpch
    echo "\n" >> microbench_tpch
done
