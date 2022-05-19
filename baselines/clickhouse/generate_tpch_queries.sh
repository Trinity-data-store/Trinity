mkdir -p query_tpch_template1
python3 query_new.py 1 100 >> query_tpch_template1/part_1 &
python3 query_new.py 1 100 >> query_tpch_template1/part_2 &
python3 query_new.py 1 100 >> query_tpch_template1/part_3 &
python3 query_new.py 1 100 >> query_tpch_template1/part_4 &
python3 query_new.py 1 100 >> query_tpch_template1/part_5 &
# python3 query.py 1 100 >> query_tpch_template1/part_6 &
# python3 query.py 1 100 >> query_tpch_template1/part_7 &
# python3 query.py 1 100 >> query_tpch_template1/part_8 &
# python3 query.py 1 100 >> query_tpch_template1/part_9 &
# python3 query.py 1 100 >> query_tpch_template1/part_10 &

# python3 query.py 2 500 >> query_tpch_template2_5node