cd build
make

TPCH=1
GITHUB=2
NYC=3

echo "*** tpch ***"
sh /proj/trinity-PG0/Trinity/scripts/trinity_start.sh $TPCH
sleep 60
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH $i" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityTPCH 0
sleep 300
sh /proj/trinity-PG0/Trinity/scripts/trinity_end.sh
sleep 60

echo "*** github ***"
sh /proj/trinity-PG0/Trinity/scripts/trinity_start.sh $GITHUB
sleep 60
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub $i" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityGithub 0
sleep 300
sh /proj/trinity-PG0/Trinity/scripts/trinity_end.sh
sleep 60

echo "*** nyc ***"
sh /proj/trinity-PG0/Trinity/scripts/trinity_start.sh $NYC
sleep 60
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "/proj/trinity-PG0/Trinity/build/librpc/TrinityNYC $i" &
done
/proj/trinity-PG0/Trinity/build/librpc/TrinityNYC 0
sleep 300
sh /proj/trinity-PG0/Trinity/scripts/trinity_end.sh
sleep 60
