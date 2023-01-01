
cd /proj/trinity-PG0/Trinity/build/
make

for i in {12..16}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "/proj/trinity-PG0/Trinity/build/librpc/MDTrieShardServer 10.10.1.$i 20" &
done


