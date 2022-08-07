for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "sudo pkill python3" &
done

clickhouse-client --database=default --query="DROP TABLE IF EXISTS nyc_taxi" && clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS nyc_taxi (pkey UInt32, pickup_date UInt32, dropoff_date UInt32, pickup_longitude Float32, pickup_latitude Float32, dropoff_longitude Float32, dropoff_latitude Float32, passenger_count UInt32, trip_distance UInt32, fare_amount UInt32, extra UInt32, mta_tax UInt32, tip_amount UInt32, tolls_amount UInt32, improvement_surcharge UInt32, total_amount UInt32
) ENGINE = Distributed(test_trinity, default, nyc_taxi, rand());"

for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'clickhouse-client --database=default --query="DROP TABLE IF EXISTS nyc_taxi"; && clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS nyc_taxi (pkey UInt32, pickup_date UInt32, dropoff_date UInt32, pickup_longitude Float32, pickup_latitude Float32, dropoff_longitude Float32, dropoff_latitude Float32, passenger_count UInt32, trip_distance UInt32, fare_amount UInt32, extra UInt32, mta_tax UInt32, tip_amount UInt32, tolls_amount UInt32, improvement_surcharge UInt32, total_amount UInt32) ENGINE = Distributed(test_trinity, default, nyc_taxi, rand());"'
done

for i in {10..14}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'clickhouse-client --database=default --query="DROP TABLE IF EXISTS nyc_taxi" && clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS nyc_taxi (pkey UInt32, pickup_date UInt32, dropoff_date UInt32, pickup_longitude Float32, pickup_latitude Float32, dropoff_longitude Float32, dropoff_latitude Float32, passenger_count UInt32, trip_distance UInt32, fare_amount UInt32, extra UInt32, mta_tax UInt32, tip_amount UInt32, tolls_amount UInt32, improvement_surcharge UInt32, total_amount UInt32) Engine = MergeTree ORDER BY (pkey)"'
done

sleep 5

for i in {1..4}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/clickhouse/python/query_nyc_throughput.py $i" &
done

python3 /proj/trinity-PG0/Trinity/baselines/clickhouse/python/query_nyc_throughput.py 0