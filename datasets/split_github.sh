sudo mkdir -p /mntData2/github_split_10
cd /mntData2/github_split_10
sudo rm ./*
sudo split -l 20000000 /mntData2/github/github_events_processed_9.csv --numeric-suffixes
sudo split -l 82805630 /mntData2/github/github_events_processed.csv --numeric-suffixes

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