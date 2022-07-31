#!/bin/bash

cd /proj/trinity-PG0/Trinity/build/
make

for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithubInsert $i >> /proj/trinity-PG0/Trinity/results/trinity_github_insert_throughput" &
done

/proj/trinity-PG0/Trinity/build/librpc/TrinityGithubInsert 0 >> /proj/trinity-PG0/Trinity/results/trinity_github_insert_throughput

