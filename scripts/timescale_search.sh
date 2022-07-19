# RUN insertion!

for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/timescaleDB/python/search_insert_tpch_new.py $i no_insert" &
done

python3 /proj/trinity-PG0/Trinity/baselines/timescaleDB/python/search_insert_tpch_new.py 0 no_insert