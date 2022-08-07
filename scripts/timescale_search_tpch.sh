# RUN insertion!

# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "sudo pkill python3" &
# done

# for i in {10..14}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'bash /proj/trinity-PG0/Trinity/scripts/timescale_truncate_tpch.sh'
# done

# sleep 10

# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/timescaleDB/python/insert_tpch_for_query_throughput.py $i" &
# done

# python3 /proj/trinity-PG0/Trinity/baselines/timescaleDB/python/insert_tpch_for_query_throughput.py 0

# echo "sleep 20"
# sleep 20


for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/timescaleDB/python/search_insert_tpch_new.py $i no_insert" &
done

python3 /proj/trinity-PG0/Trinity/baselines/timescaleDB/python/search_insert_tpch_new.py 0 no_insert