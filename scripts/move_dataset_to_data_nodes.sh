# for i in {12..16}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "sudo chmod 775 /mntData/"
#     sudo scp -i /proj/trinity-PG0/Trinity/scripts/key -o StrictHostKeyChecking=no -r /mntData/nyc_split_10 Ziming@10.10.1.$i:/mntData &
# done

for i in {10..10}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "sudo chmod 775 /mntData/"
    sudo scp -i /proj/trinity-PG0/Trinity/scripts/key -o StrictHostKeyChecking=no -r /mntData/nyc_taxi_processed_675.csv Ziming@10.10.1.$i:/mntData &
done