cd /proj/trinity-PG0/Trinity/build
make -j

base_dir=../baselines/clickhouse

for i in 2 3
do
    echo $base_dir/query_tpch_T"$i"_range0.10_rerun_converted
    echo $base_dir/query_tpch_T"$i"_range0.10_trinity
    librpc/TrinityTPCHMacro 1 $base_dir/query_tpch_T"$i"_range0.10_rerun_converted $base_dir/query_tpch_T"$i"_range0.10_trinity
done
