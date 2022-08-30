bash /proj/trinity-PG0/Trinity/scripts/initialize_node.sh
sudo bash /proj/trinity-PG0/Trinity/baselines/timescaleDB/initialization_script.sh
sudo bash /proj/trinity-PG0/Trinity/baselines/clickhouse/initialization_script.sh
sudo bash /proj/trinity-PG0/Trinity/baselines/aerospike/initialization_script.sh

for i in {3..16}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$i "sudo bash /proj/trinity-PG0/Trinity/scripts/initialize_node.sh && sudo bash /proj/trinity-PG0/Trinity/baselines/timescaleDB/initialization_script.sh && sudo bash /proj/trinity-PG0/Trinity/baselines/clickhouse/initialization_script.sh && sudo bash /proj/trinity-PG0/Trinity/baselines/aerospike/initialization_script.sh" &
done

bash /proj/trinity-PG0/Trinity/scripts/trinity_initialize.sh