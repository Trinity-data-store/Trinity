# sudo split -l 129891965 /mntData2/nyc_taxi/nyc_taxi_processed_ch.csv --numeric-suffixes


# 844000000


# sudo split -l 129891965 /mntData2/nyc_taxi/nyc_taxi_processed_ch.csv --numeric-suffixes


# Total: 675200000
# sudo split -l 16880000 /mntData2/nyc_taxi/nyc_taxi_processed_675.csv --numeric-suffixes

cd /mntData/nyc_split_10/
sudo rm ./*
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