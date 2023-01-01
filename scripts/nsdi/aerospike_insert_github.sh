# for i in {1..9}
# do
#     ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "sudo pkill python3" &
# done

# i=10
# ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) 'asinfo -v "truncate:namespace=macro_bench;set=github_macro"'

# sleep 10

# Bottlenecked
for i in {1..9}
do
    ssh -o StrictHostKeyChecking=no -i /proj/trinity-PG0/Trinity/scripts/key -l Ziming 10.10.1.$(($i + 2)) "python3 /proj/trinity-PG0/Trinity/baselines/aerospike/python/insert_github.py $i" &
done

python3 /proj/trinity-PG0/Trinity/baselines/aerospike/python/insert_github.py 0
