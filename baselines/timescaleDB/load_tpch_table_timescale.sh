for i in {1..60}
do
    timescaledb-parallel-copy --db-name tpch_macro --table tpch_macro \
        --file /mntData/tpch_split/x$i --copy-options "CSV" \
        --workers 20 --reporting-period 30s --batch-size 200000 \
        --connection "host=localhost user=postgres password=adifficultpassword sslmode=disable"
done