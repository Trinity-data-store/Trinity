for i in {10..14}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'bash /proj/trinity-PG0/Trinity/scripts/truncate_timescale_table.sh'
done

sleep 10

for i in {1..5}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/timescaleDB/python/insert_tpch_new.py $i" &
done

python3 /proj/trinity-PG0/Trinity/baselines/timescaleDB/python/insert_tpch_new.py 0