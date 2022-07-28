#!/bin/bash

cd /mntData/nyc_split_10

for i in {3..11}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "sudo chmod 775 /mntData/ && sudo rm -r /mntData/nyc_split_10"
    sudo scp -i /proj/trinity-PG0/Trinity/scripts/key -o StrictHostKeyChecking=no -r /mntData/nyc_split_10 Ziming@10.10.1.$i:/mntData &
done