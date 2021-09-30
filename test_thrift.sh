#!/usr/bin/env bash

for server_index in {9090..9099}
do
    /home/ziming/md-trie/build/librpc/MDTrieShardServer $server_index &
done

/home/ziming/md-trie/build/librpc/MDTrieShardTest 36

kill $(jobs -p)