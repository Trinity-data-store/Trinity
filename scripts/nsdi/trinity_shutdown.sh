for i in {12..16}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "sudo pkill -f /proj/trinity-PG0/Trinity/build/librpc/MDTrieShardServer"
done

for i in {2..11}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "sudo pkill -f /proj/trinity-PG0/Trinity/build/librpc/"
done
