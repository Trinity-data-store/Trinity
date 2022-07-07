sudo mkdir -p /mntData/tpch_split
cd /mntData/tpch_split
# sudo split -l 25000000 /mntData2/tpch/data_300/tpch_processed_shortened.csv --numeric-suffixes
sudo split -l 16666667 /mntData2/tpch/data_500/tpch_processed_1B.csv --numeric-suffixes

sudo mkdir -p /mntData/tpch_split
cd /mntData/tpch_split
sudo split -l 16666667 /mntData2/tpch/data_500/orders_lineitem_merged_indexed.csv --numeric-suffixes

cd /mntData/tpch_split
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

for i in {0..300}
do
    if [ ! -d "thrift"]; then
        echo "not exist: $i"
    fi
done


# for i in {101..210}
# do
#     sudo mv "x$(( 9000 + $i ))" "x$(( 90 + $i ))"
# done
