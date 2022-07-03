
# Clickhouse TPCH (insert Data)
# sudo split -l 277777 /mntData/tpch_split/x59 --numeric-suffixes

for i in $(seq 1 58);
do
    echo $i
    cat /mntData/tpch_split/x$i | clickhouse-client --query="INSERT INTO tpch_macro FORMAT CSV";
done