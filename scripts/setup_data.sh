#!/bin/bash

data_dir=/mntData2
local_dir=/mntData
num_clients=10

sudo chmod 775 $local_dir
cp -r $data_dir/dataset_csv $local_dir
cp -r $data_dir/queries $local_dir

for i in {3..$((2 + $num_clients))}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "sudo chmod 775 $local_dir"
    sudo scp -i /proj/trinity-PG0/Trinity/scripts/key -o StrictHostKeyChecking=no -r $local_dir/dataset_csv Ziming@10.10.1.$i:$local_dir &
    sudo scp -i /proj/trinity-PG0/Trinity/scripts/key -o StrictHostKeyChecking=no -r $local_dir/queries Ziming@10.10.1.$i:$local_dir &
done
