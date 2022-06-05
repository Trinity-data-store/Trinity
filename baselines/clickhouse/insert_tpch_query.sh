
cat /mntData/tpch_split/x3 | clickhouse-client --query="INSERT INTO tpch_macro FORMAT CSV"
python3 try_out_query.py 1