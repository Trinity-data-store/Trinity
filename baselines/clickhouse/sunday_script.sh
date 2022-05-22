cd /proj/trinity-PG0/Trinity/baselines/clickhouse

python3 query_new.py 2 122 >> query_tpch_T2_range0.10_new
python3 re_run_query.py query_tpch_T2_range0.10_new >> query_tpch_T2_range0.10_rerun
python3 convert_query_format.py query_tpch_T2_range0.10_rerun

python3 query_new.py 5 198 >> query_tpch_T5_range0.10
python3 re_run_query.py query_tpch_T5_range0.10 >> query_tpch_T5_range0.10_rerun
python3 convert_query_format.py query_tpch_T5_range0.10_rerun
