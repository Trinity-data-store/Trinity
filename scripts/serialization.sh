#!/bin/sh

cd /home/ziming/md-trie/build/
make

echo "**** tpch_bench  $i's iteration started! ****" >> ../results_log/deserialize_tpch.log
libmdtrie/tpch_bench 2 >> ../results_log/deserialize_tpch.log
echo "\n" >> ../results_log/deserialize_tpch.log
