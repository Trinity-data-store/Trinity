# python3 re_run_query.py query_tpch_T1_R100
# python3 re_run_query.py query_tpch_T4_R100

# python3 re_run_query.py query_tpch_T5_R100

# librpc/TrinityTPCHMacro 0 ../baselines/clickhouse/query_tpch_T2_range0.10_rerun_converted ../baselines/clickhouse/query_tpch_T2_range0.10_trinity

cd /proj/trinity-PG0/Trinity/build
librpc/TrinityTPCHMacro 1
sleep 30

librpc/TrinityTPCHMacro 0 ../baselines/clickhouse/query_tpch_T1_R100_rerun_converted ../baselines/clickhouse/query_tpch_T1_R100_trinity
librpc/TrinityTPCHMacro 0 ../baselines/clickhouse/query_tpch_T4_R100_rerun_converted ../baselines/clickhouse/query_tpch_T4_R100_trinity
librpc/TrinityTPCHMacro 0 ../baselines/clickhouse/query_tpch_T5_R100_rerun_converted ../baselines/clickhouse/query_tpch_T5_R100_trinity