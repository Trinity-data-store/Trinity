for i in {10..14}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'bash /proj/trinity-PG0/Trinity/scripts/timescale_truncate_tpch.sh'
done

python3 /proj/trinity-PG0/Trinity/baselines/timescaleDB/python/insert_lookup_latency_tpch.py