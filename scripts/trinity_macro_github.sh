#!/bin/bash

TPCH=1
GITHUB=2
NYC=3

cd /proj/trinity-PG0/Trinity/build/
make

echo "*** Github ***"
bash /proj/trinity-PG0/Trinity/scripts/trinity_start.sh $GITHUB
sleep 20

echo "*** Github insert ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d $i -b insert" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b insert
wait
sleep 120

echo "*** Github lookup ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d $i -b lookup" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b lookup
wait
sleep 120

echo "*** Github query latency ***"
/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b query_latency
wait
sleep 20


echo "*** Github query throughput ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d $i -b query_throughput" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b query_throughput
wait
sleep 120

echo "*** Github lookup mixed ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d $i -b lookup_mixed" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b lookup_mixed
wait
sleep 120

echo "*** Github search mixed ***"
for i in {1..1}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d $i -b search_mixed" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b search_mixed
wait
sleep 120

echo "*** Github insert mixed ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d $i -b insert_mixed" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b insert_mixed
wait
sleep 120

bash /proj/trinity-PG0/Trinity/scripts/trinity_end.sh
sleep 20
