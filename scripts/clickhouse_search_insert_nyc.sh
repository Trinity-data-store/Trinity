for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "sudo pkill python3" &
done

# bash /proj/trinity-PG0/Trinity/scripts/clickhouse_start.sh

sleep 5

clickhouse-client --database=default --query="DROP TABLE IF EXISTS nyc_taxi" && clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS nyc_taxi (pkey UInt32, pickup_date UInt32, dropoff_date UInt32, pickup_lon Float32, pickup_lat Float32, dropoff_lon Float32, dropoff_lat Float32, passenger_cnt UInt32, trip_dist UInt32, fare_amt UInt32, extra UInt32, mta_tax UInt32, tip_amt UInt32, tolls_amt UInt32, impt_sur UInt32, total_amt UInt32
) ENGINE = Distributed(test_trinity, default, nyc_taxi, rand());"

for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'clickhouse-client --database=default --query="DROP TABLE IF EXISTS nyc_taxi"; && clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS nyc_taxi (pkey UInt32, pickup_date UInt32, dropoff_date UInt32, pickup_lon Float32, pickup_lat Float32, dropoff_lon Float32, dropoff_lat Float32, passenger_cnt UInt32, trip_dist UInt32, fare_amt UInt32, extra UInt32, mta_tax UInt32, tip_amt UInt32, tolls_amt UInt32, impt_sur UInt32, total_amt UInt32) ENGINE = Distributed(test_trinity, default, nyc_taxi, rand());"'
done

for i in {10..14}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'clickhouse-client --database=default --query="DROP TABLE IF EXISTS nyc_taxi" && clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS nyc_taxi (pkey UInt32, pickup_date UInt32, dropoff_date UInt32, pickup_lon Float32, pickup_lat Float32, dropoff_lon Float32, dropoff_lat Float32, passenger_cnt UInt32, trip_dist UInt32, fare_amt UInt32, extra UInt32, mta_tax UInt32, tip_amt UInt32, tolls_amt UInt32, impt_sur UInt32, total_amt UInt32) Engine = MergeTree ORDER BY (pkey)"'
done

sleep 5

for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/clickhouse/python/search_insert_nyc.py $i" &
done

python3 /proj/trinity-PG0/Trinity/baselines/clickhouse/python/search_insert_nyc.py 0