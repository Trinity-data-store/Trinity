i=10
ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'asinfo -v "truncate:namespace=tpch;set=tpch_macro"'

sleep 5

# Bottlenecked
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/aerospike/python/insert_tpch.py $i" &
done

python3 /proj/trinity-PG0/Trinity/baselines/aerospike/python/insert_tpch.py 0