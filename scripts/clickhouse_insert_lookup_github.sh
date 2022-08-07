clickhouse-client --database=default --query="DROP TABLE IF EXISTS github_events" && clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS github_events (pkey UInt32, events_count UInt32, authors_count UInt32, forks UInt32, stars UInt32, issues UInt32, pushes UInt32, pulls UInt32, downloads UInt32, adds UInt32, dels UInt32, add_del_ratio Float32, start_date UInt32, end_date UInt32) ENGINE = Distributed(test_trinity, default, github_events, rand());"

for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'clickhouse-client --database=default --query="DROP TABLE IF EXISTS github_events"; && clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS github_events (pkey UInt32, events_count UInt32, authors_count UInt32, forks UInt32, stars UInt32, issues UInt32, pushes UInt32, pulls UInt32, downloads UInt32, adds UInt32, dels UInt32, add_del_ratio Float32, start_date UInt32, end_date UInt32) ENGINE = Distributed(test_trinity, default, github_events, rand());"'
done

for i in {10..14}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'clickhouse-client --database=default --query="DROP TABLE IF EXISTS github_events" && clickhouse-client --database=default --query="CREATE TABLE IF NOT EXISTS github_events (pkey UInt32, events_count UInt32, authors_count UInt32, forks UInt32, stars UInt32, issues UInt32, pushes UInt32, pulls UInt32, downloads UInt32, adds UInt32, dels UInt32, add_del_ratio Float32, start_date UInt32, end_date UInt32) Engine = MergeTree ORDER BY (pkey)"'
done

sleep 5

# for i in {0..8}
# do
#     echo "inserting datasets: $i"
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "cat /mntData/tpch_split_10/x$i | clickhouse-client --query=\"INSERT INTO tpch_macro FORMAT CSV\""
# done

# sleep 5

for i in {1..4}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/clickhouse/python/insert_lookup_github.py $i" &
done

python3 /proj/trinity-PG0/Trinity/baselines/clickhouse/python/insert_lookup_github.py 0