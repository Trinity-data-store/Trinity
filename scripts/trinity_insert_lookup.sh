#!/bin/bash
# Run in on CLOUDLAB terminal!!

cd /proj/trinity-PG0/Trinity/build/
make

for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCHInsertLookup $i" &
done

/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCHInsertLookup 0

cd /proj/trinity-PG0/Trinity
# ulimit -n 100000
# cd /proj/trinity-PG0/Trinity/build
