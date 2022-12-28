
cd /proj/trinity-PG0/Trinity/build/
make

TPCH=1
GITHUB=2
NYC=3
DATASET=$1

for i in {12..16}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "/proj/trinity-PG0/Trinity/build/librpc/MDTrieShardServer -i 10.10.1.$i -s 20 -d $DATASET" &
done


