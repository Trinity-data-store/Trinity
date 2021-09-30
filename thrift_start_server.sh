#!/usr/bin/env bash

for server_index in {9090..9125}
do
    /home/ziming/md-trie/build/librpc/MDTrieShardServer $server_index &
done