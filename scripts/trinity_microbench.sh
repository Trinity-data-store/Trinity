cd /proj/trinity-PG0/Trinity/build
make

# echo "trinity - TPCH"
# /proj/trinity-PG0/Trinity/build/libmdtrie/tpch_bench >> tpch_bench_log

# echo "trinity - Github"
# /proj/trinity-PG0/Trinity/build/libmdtrie/github_bench >> github_bench_log

echo "trinity - NYC"
/proj/trinity-PG0/Trinity/build/libmdtrie/nyc_bench >> nyc_bench_log

# cd baselines/phtree/phtree-cpp/build/
# make

# echo "phtree - TPCH"
# ./microbench/micro_tpch >> phtree_tpch_log

# echo "phtree - Github"
# ./microbench/micro_github >> phtree_github_log

# echo "phtree - NYC"
# ./microbench/micro_nyc >> phtree_nyc_log