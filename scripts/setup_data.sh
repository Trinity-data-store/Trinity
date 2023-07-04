#!/bin/bash

sudo chmod 775 /mntData
cp -r /mntData2/dataset_csv /mntData
cp -r /mntData2/queries /mntData

for i in {3..12}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "sudo chmod 775 /mntData/"
    sudo scp -i /proj/trinity-PG0/Trinity/scripts/key -o StrictHostKeyChecking=no -r /mntData/dataset_csv Ziming@10.10.1.$i:/mntData &
    sudo scp -i /proj/trinity-PG0/Trinity/scripts/key -o StrictHostKeyChecking=no -r /mntData/queries Ziming@10.10.1.$i:/mntData &
done
