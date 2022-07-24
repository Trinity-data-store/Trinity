#!/bin/bash

# if [ ! -d "/mntData/tpch_split_600" ]; then
#     sudo mkdir -p /mntData/tpch_split_600
#     cd /mntData/tpch_split_600
#     sudo split -l 1666667 /mntData2/tpch/data_300/tpch_processed_1B.csv --numeric-suffixes
#     for i in {0..9}
#     do
#         sudo mv x0$i x$i 
#     done
    
#     for i in {90..599}
#     do
#         sudo mv "x$(( 8910 + $i ))" "x$(( $i ))"
#     done
# fi

# sudo mkdir -p /mntData/tpch_split_10
# cd /mntData/tpch_split_10
# sudo split -l 100000000 /mntData2/tpch/data_300/tpch_processed_1B.csv --numeric-suffixes
# for i in {0..9}
# do
#     sudo mv x0$i x$i
# done

for i in {3..11}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "sudo chmod 775 /mntData/"
    sudo scp -i /proj/trinity-PG0/Trinity/scripts/key -o StrictHostKeyChecking=no -r /mntData/nyc_split_10 Ziming@10.10.1.$i:/mntData &
done