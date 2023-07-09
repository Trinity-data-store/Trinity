#!/bin/bash

TPCH=1
GITHUB=2
NYC=3

cd /proj/trinity-PG0/Trinity/build/
make

echo "*** TPCH ***"
bash /proj/trinity-PG0/Trinity/scripts/trinity_start.sh $TPCH
sleep 5

echo "*** TPCH insert ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d $i -b insert" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b insert
wait       # waits for all child processes
sleep 120

echo "*** TPCH lookup ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d $i -b lookup" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b lookup
sleep 120

echo "*** TPCH query latency ***"
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b query_latency
sleep 20

echo "*** TPCH lookup throughput ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d $i -b query_throughput" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b query_throughput
sleep 120


echo "*** TPCH lookup mixed ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d $i -b lookup_mixed" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b lookup_mixed
wait       # waits for all child processes
sleep 120

echo "*** TPCH search mixed ***"
for i in {1..1} # It will otherwise overwhelm the server
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d $i -b search_mixed" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b search_mixed
wait       # waits for all child processes
sleep 120

echo "*** TPCH insert mixed ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d $i -b insert_mixed" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b insert_mixed
wait       # waits for all child processes
sleep 120

bash /proj/trinity-PG0/Trinity/scripts/trinity_end.sh
sleep 20
