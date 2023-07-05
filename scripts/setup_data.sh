#!/bin/bash

data_dir=/mntData2
local_dir=/mntData
num_clients=10

sudo chmod 775 $local_dir
cp -a $data_dir/dataset_csv/. $local_dir/

for i in {3..$((2 + $num_clients))}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "sudo chmod 775 $local_dir"

    for repo in 'tpch_split' 'github_split' 'nyc_split'
    do
        ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "sudo mkdir $local_dir/$repo && sudo chmod 775 $local_dir/$repo"
        sudo scp -i /proj/trinity-PG0/Trinity/scripts/key -o StrictHostKeyChecking=no -r $local_dir/$repo/x0$(($i - 3)) Ziming@10.10.1.$i:$local_dir/$repo &
    done
done
