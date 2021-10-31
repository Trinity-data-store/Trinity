#!/usr/bin/env bash

for server_index in {9090..9161}
# for server_index in {9090}
do
    # echo $((server_index - 9090))
    taskset -c $((server_index - 9090)) /home/ziming/md-trie/build/librpc/MDTrieShardServer $server_index &
done