# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "sudo pkill python3" &
# done


# sleep 5

# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/aerospike/python/insert_github_for_search_throughput.py $i" &
# done

# python3 /proj/trinity-PG0/Trinity/baselines/aerospike/python/insert_github_for_search_throughput.py 0

# sleep 15

# Bottlenecked
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/aerospike/python/insert_lookup_github.py $i" &
done

python3 /proj/trinity-PG0/Trinity/baselines/aerospike/python/insert_lookup_github.py 0