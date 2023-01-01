for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "sudo pkill python3" &
done

sleep 5

clickhouse-client --database=default --query="DROP TABLE IF EXISTS tpch_macro" && clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS tpch_macro (ID UInt32, QUANTITY UInt32, EXTENDEDPRICE UInt32, DISCOUNT UInt32, TAX UInt32, SHIPDATE UInt32, COMMITDATE UInt32, RECEIPTDATE UInt32, TOTALPRICE UInt32, ORDERDATE UInt32) ENGINE = Distributed(test_trinity, default, tpch_macro, rand());"

for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'clickhouse-client --database=default --query="DROP TABLE IF EXISTS tpch_macro"; && clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS tpch_macro (ID UInt32, QUANTITY UInt32, EXTENDEDPRICE UInt32, DISCOUNT UInt32, TAX UInt32, SHIPDATE UInt32, COMMITDATE UInt32, RECEIPTDATE UInt32, TOTALPRICE UInt32, ORDERDATE UInt32) ENGINE = Distributed(test_trinity, default, tpch_macro, rand());"'
done

for i in {10..14}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'clickhouse-client --database=default --query="DROP TABLE IF EXISTS tpch_macro"; && clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS tpch_macro (ID UInt32, QUANTITY UInt32, EXTENDEDPRICE UInt32, DISCOUNT UInt32, TAX UInt32, SHIPDATE UInt32, COMMITDATE UInt32, RECEIPTDATE UInt32, TOTALPRICE UInt32, ORDERDATE UInt32) Engine = MergeTree ORDER BY (ID)";'
done

sleep 5

for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/clickhouse/python/search_insert_tpch.py $i" &
done

python3 /proj/trinity-PG0/Trinity/baselines/clickhouse/python/search_insert_tpch.py 0