sudo mkdir -p /mntData/github_split_10
cd /mntData/github_split_10
sudo split -l 82805630 /mntData2/github/github_events_processed_9.csv --numeric-suffixes
sudo mv x00 x0
sudo mv x01 x1
sudo mv x02 x2
sudo mv x03 x3
sudo mv x04 x4 
sudo mv x05 x5 
sudo mv x06 x6 
sudo mv x07 x7 
sudo mv x08 x8 
sudo mv x09 x9 
echo "finished github split"

sudo mkdir -p /mntData/nyc_split_10
cd /mntData/nyc_split_10
sudo split -l 67520000 /mntData2/nyc_taxi/nyc_taxi_processed_675.csv --numeric-suffixes
sudo mv x00 x0
sudo mv x01 x1
sudo mv x02 x2
sudo mv x03 x3
sudo mv x04 x4 
sudo mv x05 x5 
sudo mv x06 x6 
sudo mv x07 x7 
sudo mv x08 x8 
sudo mv x09 x9 
echo "finished nyc split"

sudo mkdir -p /mntData/tpch_split_10
cd /mntData/tpch_split_10
sudo split -l 100000000 /mntData2/tpch/data_300/tpch_processed_1B.csv --numeric-suffixes
sudo mv x00 x0
sudo mv x01 x1
sudo mv x02 x2
sudo mv x03 x3
sudo mv x04 x4 
sudo mv x05 x5 
sudo mv x06 x6 
sudo mv x07 x7 
sudo mv x08 x8 
sudo mv x09 x9 
echo "finished tpch split"

bash /proj/trinity-PG0/Trinity/scripts/move_github_dataset_to_other_nodes.sh
sleep 60

bash /proj/trinity-PG0/Trinity/scripts/move_tpch_dataset_to_other_nodes.sh
sleep 60

bash /proj/trinity-PG0/Trinity/scripts/move_nyc_dataset_to_other_nodes.sh
sleep 60