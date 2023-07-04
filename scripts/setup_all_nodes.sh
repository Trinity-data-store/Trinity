#!/bin/bash

bash /proj/trinity-PG0/Trinity/scripts/setup_one_node.sh

for i in {3..16}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "bash /proj/trinity-PG0/Trinity/scripts/setup_one_node.sh"
done