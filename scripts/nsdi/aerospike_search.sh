# IMPORTANT!
# IMPORTANT!
# IMPORTANT!
# IMPORTANT!
# RUN aerospike insert first!

# Bottlenecked
for i in {1..3}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/aerospike/python/search_insert_tpch.py $i no_insert" &
done

python3 /proj/trinity-PG0/Trinity/baselines/aerospike/python/search_insert_tpch.py 0 no_insert