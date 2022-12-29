#!/bin/bash

TPCH=1
GITHUB=2
NYC=3

cd /proj/trinity-PG0/Trinity/build/
make

echo "*** TPCH ***"
bash /proj/trinity-PG0/Trinity/scripts/trinity_start.sh $TPCH
sleep 20

echo "*** TPCH insert ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d $i -b insert" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b insert
sleep 60

echo "*** TPCH lookup ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d $i -b lookup" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b lookup
sleep 60

echo "*** TPCH query latency ***"
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b query_latency
sleep 20

echo "*** TPCH lookup throughput ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d $i -b query_throughput" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b query_throughput
sleep 60


echo "*** TPCH lookup mixed ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d $i -b lookup_mixed" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b lookup_mixed
sleep 60

echo "*** TPCH search mixed ***"
for i in {1..1} # It will otherwise overwhelm the server
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d $i -b search_mixed" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b search_mixed
sleep 60

echo "*** TPCH insert mixed ***"
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d $i -b insert_mixed" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH -d 0 -b insert_mixed
sleep 60

bash /proj/trinity-PG0/Trinity/scripts/trinity_end.sh
sleep 20

# echo "*** Github ***"
# bash /proj/trinity-PG0/Trinity/scripts/trinity_start.sh $GITHUB
# sleep 20

# echo "*** Github insert ***"
# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d $i -b insert" &
# done
# /proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b insert
# sleep 60

# echo "*** Github lookup ***"
# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d $i -b lookup" &
# done
# /proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b lookup
# sleep 60

# echo "*** Github query latency ***"
# /proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b query_latency
# sleep 20


# echo "*** Github query throughput ***"
# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d $i -b query_throughput" &
# done
# /proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b query_throughput
# sleep 60

# echo "*** Github lookup mixed ***"
# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d $i -b lookup_mixed" &
# done
# /proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b lookup_mixed
# sleep 60

# echo "*** Github search mixed ***"
# for i in {1..1}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d $i -b search_mixed" &
# done
# /proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b search_mixed
# sleep 60

# echo "*** Github insert mixed ***"
# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d $i -b insert_mixed" &
# done
# /proj/trinity-PG0/Trinity/build/librpc/TrinityGithub -d 0 -b insert_mixed
# sleep 60

# bash /proj/trinity-PG0/Trinity/scripts/trinity_end.sh
# sleep 20


# echo "*** NYC ***"
# bash /proj/trinity-PG0/Trinity/scripts/trinity_start.sh $NYC
# sleep 20

# echo "*** NYC insert ***"
# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityNYC -d $i -b insert" &
# done
# /proj/trinity-PG0/Trinity/build/librpc/TrinityNYC -d 0 -b insert
# sleep 60

# echo "*** NYC lookup ***"
# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityNYC -d $i -b lookup" &
# done
# /proj/trinity-PG0/Trinity/build/librpc/TrinityNYC -d 0 -b lookup
# sleep 60

# echo "*** NYC query latency ***"
# /proj/trinity-PG0/Trinity/build/librpc/TrinityNYC -d 0 -b query_latency
# sleep 20

# echo "*** NYC query throughput ***"
# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityNYC -d $i -b query_throughput" &
# done
# /proj/trinity-PG0/Trinity/build/librpc/TrinityNYC -d 0 -b query_throughput
# sleep 60

# echo "*** NYC lookup mixed ***"
# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityNYC -d $i -b lookup_mixed" &
# done
# /proj/trinity-PG0/Trinity/build/librpc/TrinityNYC -d 0 -b lookup_mixed
# sleep 60

# echo "*** NYC search mixed ***"
# for i in {1..1}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityNYC -d $i -b search_mixed" &
# done
# /proj/trinity-PG0/Trinity/build/librpc/TrinityNYC -d 0 -b search_mixed
# sleep 60

# echo "*** NYC insert mixed ***"
# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityNYC -d $i -b insert_mixed" &
# done
# /proj/trinity-PG0/Trinity/build/librpc/TrinityNYC -d 0 -b insert_mixed
# sleep 60

# bash /proj/trinity-PG0/Trinity/scripts/trinity_end.sh
# sleep 20