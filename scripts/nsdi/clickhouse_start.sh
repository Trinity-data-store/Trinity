for i in {1..14}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "sudo cp /proj/trinity-PG0/Trinity/baselines/clickhouse/clickhouse_config.xml /etc/clickhouse-server/config.xml && sudo cp /proj/trinity-PG0/Trinity/baselines/clickhouse/clickhouse_users.xml /etc/clickhouse-server/users.xml && sudo service clickhouse-server restart" &
done

sudo cp /proj/trinity-PG0/Trinity/baselines/clickhouse/clickhouse_config.xml /etc/clickhouse-server/config.xml && sudo cp /proj/trinity-PG0/Trinity/baselines/clickhouse/clickhouse_users.xml /etc/clickhouse-server/users.xml && sudo service clickhouse-server restart