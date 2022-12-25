for i in {1..4}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/clickhouse/python/lookup_github.py $i" &
done

python3 /proj/trinity-PG0/Trinity/baselines/clickhouse/python/lookup_github.py 0