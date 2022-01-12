#!/bin/sh

cd /home/ziming/md-trie/build/

NUM_REPEATS=1

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** fs_bench $i's iteration started! ****" >> microbench_output
    libmdtrie/fs_bench >> microbench_output
done

echo "\n" >> microbench_output

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** osm_bench  $i's iteration started! ****" >> microbench_output
    libmdtrie/osm_bench >> microbench_output
done

echo "\n" >> microbench_output

for i in $( seq 1 $NUM_REPEATS )
do
    echo "**** tpch_bench  $i's iteration started! ****" >> microbench_output
    libmdtrie/tpch_bench >> microbench_output
done

echo "\n" >> microbench_output

