
for i in `seq 20 39`
do
    cat /mntData/tpch_split/x$i | clickhouse-client --query="INSERT INTO tpch FORMAT CSV" &
done

# python3 try_out_query.py 1

exit 0

# client
clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS tpch_macro_split (ID UInt32, QUANTITY UInt8, EXTENDEDPRICE UInt32, DISCOUNT UInt8, TAX UInt8, SHIPDATE UInt32, COMMITDATE UInt32, RECEIPTDATE UInt32, TOTALPRICE UInt32, ORDERDATE UInt32) ENGINE = Distributed(test_trinity, default, tpch_macro_split, rand())";

# server
clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS tpch_macro_split (ID UInt32, QUANTITY UInt8, EXTENDEDPRICE UInt32, DISCOUNT UInt8, TAX UInt8, SHIPDATE UInt32, COMMITDATE UInt32, RECEIPTDATE UInt32, TOTALPRICE UInt32, ORDERDATE UInt32) Engine = MergeTree ORDER BY (ID)";

# drop_table
clickhouse-client --database=default --query="DROP TABLE IF EXISTS tpch_macro_split";
