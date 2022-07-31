# Bottlenecked
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/timescaleDB/python/lookup_tpch.py $i" &
done

python3 /proj/trinity-PG0/Trinity/baselines/timescaleDB/python/lookup_tpch.py 0